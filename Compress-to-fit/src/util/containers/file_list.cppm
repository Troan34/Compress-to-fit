//This file implements ConcOrderedFileList, a linked list that enables concurrent writing to file through the order of sequence numbers
//do NOT expect this stuff to be cache friendly in ANY way.
module;
#include <cassert>
export module containers:concurrent_file_queue;
import :file;
import util;
import std.compat;


template <SerializableToDisk T>
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
 * @brief First sequence in first sequence out, i.e. the data is written in the order given to #insert().
 * @tparam T Must implement write_to(ofstream&) or be trivially copyable
 *
 * @details Implemented as doubly linked list. A separate write thread is spawned.
 *
 * @note Because the list will own the elements, consider using a memory pool (e.g. pmr) as to not reallocate memory.
 */
export template<SerializableToDisk T>
class ConcOrderedFileList
{
public:
    ConcOrderedFileList(File& file)
        : file_(file)
    {
    }

    ~ConcOrderedFileList()
    {
        std::unique_lock lock{mut};
        called_writer = true;
        writer_notifier.notify_one();//write everything left
        //the thread will be destroyed automatically thanks to jthread which updates the stop token
    }

    //we really don't want to copy a concurrent container with access to a file
    ConcOrderedFileList(ConcOrderedFileList const&) = delete;
    auto operator=(ConcOrderedFileList const&) -> ConcOrderedFileList& = delete;

    ConcOrderedFileList(ConcOrderedFileList&&) noexcept = default;
    auto operator=(ConcOrderedFileList&&) noexcept -> ConcOrderedFileList& = default;

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
     *          This requirement exists as the first element you add could easily be not the first data chunk (i.e. out of order).
     *          Failure to follow this requirement will cause DEADLOCK.
     *
     */
    void insert(std::unique_ptr<T> element, size_t sequence_num)
    {
        auto* new_node = new Node<T>{std::move(element), sequence_num};

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
                Node<T>* next_node{head_};

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
            writer_notifier.notify_one();
        }
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

    auto const& file() const& noexcept
    {
        return file_;
    }
private:
    File& file_;
    size_t size_{};
    size_t expected_sequence_num_{};
    Node<T>* head_{nullptr};
    Node<T>* tail_{nullptr};
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
        //NOT thread safe by itself, will not interact with the mutex.
        //returns true if something has been written otherwise false
        auto write_func = [this]
        {
            if (!tail_)
                return false;

            if (tail_->sequence_num_ != expected_sequence_num_)//we are missing a node
                return false;

            write_to(*tail_->element_.get(), file_.get_ref_out_stream());

            auto old_tail = tail_;
            if (size_ == 1)
            {
                tail_ = nullptr;
                head_ = nullptr;
            }
            else
            {
                tail_ = tail_->prev;//new tail ptr
                tail_->next = nullptr;
            }
            delete old_tail;//delete old tail ptr

            size_--;
            expected_sequence_num_++;
            return true;
        };

        while (!stop.stop_requested())
        {
            std::unique_lock lock{mut};
            writer_notifier.wait(lock, [this]{return called_writer;});

            if (stop.stop_requested())
                break;
            called_writer = false;

            if (size_ > LEAST_NODES_FOR_WRITING)
            {
                for (auto _ : std::views::iota(0u, size_))
                    write_func();
            }
        }

        std::unique_lock lock{mut};
        while (write_func());//write the whole thing

        assert(size_ == 0 && "The file list wasn't able to clean up correctly.");
    }

};