module;
#include <cassert>
export module containers:concurrent_file_queue;
import :file;
import util;
import std.compat;

//do NOT expect this stuff to be cache friendly in ANY way.

template <SerializableToDisk T>
struct Node
{
    Node(std::unique_ptr<T> element, size_t const sequence_num)
        : element_(std::move(element)), sequence_num_(sequence_num) {}

    std::unique_ptr<T> element_;
    size_t sequence_num_;
    std::unique_ptr<Node> next{nullptr};
    Node* prev{nullptr};
};

/**
 * @brief First sequence in first sequence out, i.e. the data is written in the order given to #insert().
 * @tparam T Must implement write_to(ofstream&) or be trivially copyable
 *
 * @details Implemented as doubly linked list. A separate write thread is spawned.
 */
export template<SerializableToDisk T>
class ConcOrderedFileList
{
public:
    ConcOrderedFileList(File& file)
        : file_(file)
    {
    }

    /**
     * @brief Inserts an element to the list, which it will own. THE FIRST ELEMENT MUST HAVE SEQUENCE NUMBER == 0, check warning for additional info.
     *
     * @param element Element to be inserted
     * @param sequence_num The number of this element's sequence
     *
     * @warning THE FIRST ELEMENT THAT IS GOING TO BE WRITTEN TO FILE MUST HAVE SEQUENCE NUMBER == 0.
     *          This requirement exists as the first element you add could easily be not the first data chunk (i.e. out of order).
     *          Failure to follow this requirement will cause DEADLOCK.
     *
     * @note Because the list will own the #element , consider using a memory pool (e.g. pmr) as to not reallocate memory.
     */
    void insert(std::unique_ptr<T> element, size_t sequence_num)
    {
        {
            auto new_node = std::make_unique<Node<T>>(std::move(element), sequence_num);
            std::unique_lock lock{mut};//we could opt for a shared mutex here, profile

            size_++;
            //TODO: add the new node in the correct spot, check the scrivano for the schematics
            if (head_)
            {
                Node<T>* prev_in_sequence = nullptr;
                for (prev_in_sequence = head_.get(); prev_in_sequence->sequence_num_ != sequence_num - 1; prev_in_sequence = prev_in_sequence->prev)

                prev_in_sequence->next = new_node;
                new_node->prev = prev_in_sequence;
            }
            else
            {
                tail_ = new_node.get();//if empty, tail_ and head_ point to the same node
                head_ = std::move(new_node);
            }


        }

        called_writer = true;
        writer_notifier.notify_one();
    }

private:
    File& file_;
    size_t size_{};
    size_t expected_sequence_num_{};
    std::unique_ptr<Node<T>> head_{nullptr};
    Node<T>* tail_{nullptr};
    mutable std::mutex mut;
    std::jthread writer{write_disk};
    std::condition_variable writer_notifier;
    bool called_writer{false};


    /**
     *
     * @param stop
     * @pre tail_ != nullptr
     */
    void write_disk(std::stop_token stop)
    {
        while (!stop.stop_requested())
        {
            std::unique_lock lock{mut};
            writer_notifier.wait(lock, [this]{return called_writer;});
            called_writer = false;

            if (tail_->sequence_num != expected_sequence_num_)
                continue;

            if constexpr (std::is_trivially_copyable_v<T>)
            {
                file_.write(tail_->element_.get(), sizeof(T));
            }
            else//T implements a write_to()
            {
                tail_->element_.get()->write_to(file_.get_ref_out_stream());
            }
            expected_sequence_num_++;
        }
    }

};