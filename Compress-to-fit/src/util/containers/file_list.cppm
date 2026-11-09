/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*
*   Implements ConcOrderedFileList, a linked list that enables concurrent writing to file through the order of sequence numbers
*/

module;
#include <cassert>
export module containers:concurrent_file_queue;
import :file;
import util;
import std.compat;


template <typename T, ByteSink S>
    requires SerializableTo<T, S> ||
            (std::ranges::range<T> && SerializableTo<std::ranges::range_value_t<T>, S>)
struct Node
{
    Node(std::unique_ptr<T> element, size_t const sequence_num)
        : element_(std::move(element)), sequence_num_(sequence_num) {}

    std::unique_ptr<T> element_;
    size_t sequence_num_;
    Node* next{nullptr};
    Node* prev{nullptr};
};

//If there are less than this amount of nodes, don't bother waking the disk
constexpr int LEAST_NODES_FOR_WRITING = 10;

/**
 * @brief First sequence in first sequence out, i.e. the data is written in the order given to the 'insert()' member function.
 * @tparam T Must implement write_to(ofstream&) or be trivially copyable
 *
 * @details Implemented as doubly linked list. A separate write thread is spawned.
 *
 * @note Because the list will own the elements, consider using a memory pool (e.g. pmr) as to not reallocate memory.
 *       Do NOT expect this stuff to be cache friendly in ANY way.
 */
export template <typename T, ByteSink S>
    requires SerializableTo<T, S> ||
            (std::ranges::range<T> && SerializableTo<std::ranges::range_value_t<T>, S>)
class ConcOrderedFileList
{
public:
    explicit ConcOrderedFileList(S& sink)
        : sink_(sink)
    {
    }

    ~ConcOrderedFileList()
    {
        {
            std::lock_guard lock{mut};
            called_writer = true;
        }
        writer.request_stop();
        writer_notifier.notify_one();//write the rest
    }

    //we really don't want to copy a concurrent container with access to a file
    ConcOrderedFileList(ConcOrderedFileList const&) = delete;
    auto operator=(ConcOrderedFileList const&) -> ConcOrderedFileList& = delete;

    //TODO: I could get around to actually implementing these
    ConcOrderedFileList(ConcOrderedFileList&&) noexcept = delete;
    auto operator=(ConcOrderedFileList&&) noexcept -> ConcOrderedFileList& = delete;

    /**
     * @brief Inserts an element to the list, which it will own. THE FIRST ELEMENT MUST HAVE SEQUENCE NUMBER == 0, check warning for additional info.
     *
     * @param[in] element Element to be inserted
     * @param sequence_num The number of this element's sequence
     *
     * @pre \p element must not own a nullptr
     * @post Any reference to \p element is invalidated.
     *
     * @warning THE FIRST ELEMENT THAT IS GOING TO BE WRITTEN TO FILE MUST HAVE SEQUENCE NUMBER == 0.
     *          This requirement exists as the first element you add could easily not be the first data chunk (i.e. out of order).
     *          Failure to follow this requirement will cause DEADLOCK.
     *
     */
    void insert(std::unique_ptr<T> element, size_t sequence_num)
    {
        auto* new_node = new Node<T, S>{std::move(element), sequence_num};

        {
            std::unique_lock lock{mut};//we could opt for a shared mutex here, profile

            size_++;

            if (head_ == nullptr)
            {
                head_ = new_node;
                tail_ = new_node;
            }
            else
            {
                Node<T, S>* next_node{head_};

                //Make sure the nullptr check stays first (unless you like bugs, I don't judge)
                //If sequence_num > next_node->sequence_num_ it means that we found a spot that keeps the ascending order
                while (next_node and (sequence_num < next_node->sequence_num_)) //walks down the ptrs
                {
                    next_node = next_node->next;
                }

                if (!next_node)//We have to become the tail
                {
                    new_node->prev = tail_;
                    tail_->next = new_node;
                    tail_ = new_node;
                }
                else//we either become the head or squeeze in
                {
                    if (next_node == head_)//become head
                    {
                        new_node->next = head_;
                        head_->prev = new_node;
                        head_ = new_node;
                    }
                    else//squeeze in
                    {
                        //Make the links for the new node to its future neighbors
                        new_node->next = next_node;
                        new_node->prev = next_node->prev;

                        //Make the links from the neighbors to the new node
                        next_node->prev->next = new_node;
                        next_node->prev = new_node;
                    }
                }
            }
            called_writer = true;
        }
        writer_notifier.notify_one();
    }

    /**
     * @return Size, the number of nodes waiting to get written to file
     */
    auto size() const noexcept
    {
        std::unique_lock lock{mut};
        return size_;
    }

    /**
     * @brief Returns the next sequence number expected or missing. Can be used as a sanity check.
     * @return The expected sequence number
     */
    auto expected_sequence_num() const noexcept
    {
        std::unique_lock lock{mut};
        return expected_sequence_num_;
    }

private:
    size_t size_{};
    size_t expected_sequence_num_{};
    Node<T, S>* head_{nullptr};
    Node<T, S>* tail_{nullptr};
    S& sink_;
    bool called_writer{false};
    mutable std::mutex mut;
    std::condition_variable writer_notifier;
    std::jthread writer{&ConcOrderedFileList::write_file, this};


    /**
     * @brief Writes to file. Should be called by a thread.
     * @pre tail_ != nullptr
     */
    void write_file(std::stop_token stop)
    {

        while (true)
        {
            std::vector<std::unique_ptr<Node<T,S>>> nodes;
            {
                std::unique_lock lock{mut};

                //wait until there's something to do
                writer_notifier.wait(lock, [this]
                {
                    return called_writer;
                });
                called_writer = false;

                if (stop.stop_requested())//writes everything left
                {
                    nodes = take_writable_nodes_locked();
                    if (nodes.empty())
                        return;//nothing left, exit
                }
                else if (size_ > LEAST_NODES_FOR_WRITING)
                {
                    nodes = take_writable_nodes_locked();
                    if (nodes.empty())
                        continue;//invalid sequence, wait again
                }
                else
                {
                    continue;//not enough work yet
                }
            }
            for (auto&& node : nodes)
            {
                serialize_to(*std::move(node)->element_, sink_);
            }
        }
    }

    /**
     * @brief Unlink and pass ownership of Nodes that can be written
     * @return A vector of nodes to be written
     * @pre mut must be locked by the caller
     */
    auto take_writable_nodes_locked() -> std::vector<std::unique_ptr<Node<T, S>>>
    {
        auto nodes = std::vector<std::unique_ptr<Node<T, S>>>{};
        for (;;)
        {
            // empty  or  we are missing a node
            if (!tail_ or tail_->sequence_num_ != expected_sequence_num_)
                break;

            std::unique_ptr<Node<T, S>> node{tail_};

            if (size_ == 1)//after deleting tail, we will have no Node(s) left
            {
                tail_ = nullptr;
                head_ = nullptr;
            }
            else
            {
                tail_ = tail_->prev;//new tail ptr
                tail_->next = nullptr;
            }
            node->prev = nullptr;
            node->next = nullptr;

            size_--;
            expected_sequence_num_++;
            nodes.push_back(std::move(node));
        }
        return nodes;
    };

};