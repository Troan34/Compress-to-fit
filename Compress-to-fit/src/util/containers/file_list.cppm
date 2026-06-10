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
        //the thread will be destroyed automatically thanks to jthread and stop token
    }

    //we really don't want to copy a concurrent container with access to a file
    ConcOrderedFileList(ConcOrderedFileList const&) = delete;
    ConcOrderedFileList& operator=(ConcOrderedFileList const&) = delete;

    ConcOrderedFileList(ConcOrderedFileList&&) noexcept = default;
    ConcOrderedFileList& operator=(ConcOrderedFileList&&) noexcept = default;

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
                Node<T>* prev_node{head_};//we actually are searching for the previous node
                //Make sure the nullptr check stays first (unless you like bugs, I don't judge)
                while (prev_node and (prev_node->sequence_num_ != sequence_num - 1)) prev_node = prev_node->prev;

                if (!prev_node)//We reached the end
                {
                    new_node->next = tail_;
                    tail_->prev = new_node;
                    tail_ = new_node;
                }
                else//we found a place to squeeze in
                {
                    //set up the new node links
                    new_node->prev = prev_node;
                    new_node->next = prev_node->next;

                    //update the linking of the next and previous nodes (relative to new node)
                    if (prev_node->next) //the next node may not exist (head)
                        prev_node->next->prev = new_node;

                    prev_node->next = new_node;
                    prev_node = new_node;
                }

            }
        }

        called_writer = true;
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
    File& file_;
    size_t size_{};
    size_t expected_sequence_num_{};
    Node<T>* head_{nullptr};
    Node<T>* tail_{nullptr};
    mutable std::mutex mut;
    std::jthread writer{write_file};
    std::condition_variable writer_notifier;
    bool called_writer{false};


    /**
     * @brief Writes to file. Should be called by a thread.
     * @pre tail_ != nullptr
     */
    void write_file(std::stop_token stop)
    {
        while (!stop.stop_requested())
        {
            std::unique_lock lock{mut};
            writer_notifier.wait(lock, [this]{return called_writer;});
            called_writer = false;

            if (tail_->sequence_num != expected_sequence_num_)//we are missing a node
                continue;

            if constexpr (std::is_trivially_copyable_v<T>)
            {
                file_.write(tail_->element_, sizeof(T));
            }
            else//T implements a write_to()
            {
                tail_->element_->write_to(file_.get_ref_out_stream());
            }
            tail_ = tail_->next;//new tail ptr
            delete tail_->prev;//delete old tail ptr

            expected_sequence_num_++;
        }
    }

};