export module util:concurrent_file_queue;
import :file;
import std.compat;

template <SerializableToDisk T>
struct Node
{
    T element;
    size_t sequence_num{};
    Node* next{nullptr};
    Node* prev{nullptr};
};

/**
 * @brief First sequence in first sequence out, i.e. the data is written in the order given to #insert().
 * @tparam T Must implement write_to(ofstream&) or be trivially copyable
 *
 * @details Implemented as doubly linked list. A separate write thread is spawned.
 */
export template<SerializableToDisk T>
class ConcOrderedFileQueue
{
public:
    ConcOrderedFileQueue(File& file)
        : file_(file)
    {
    }

    void insert(T const& element, size_t sequence_num)
    {
        auto* new_node = new Node<T>{element, sequence_num, nullptr};

        std::unique_lock lock{mut};
        size_++;

        if (head_ == nullptr)
        {
            head_ = new_node;
            tail_ = new_node;
            return;
        }

        new_node->next = head_;
        head_ = new_node;

        lock.unlock();

        write_disk();
    }
private:
    File& file_;
    size_t size_{};
    size_t next_sequence_num_{};
    Node<T>* head_ = nullptr;
    Node<T>* tail_ = nullptr;
    mutable std::shared_mutex mut;


    void write_disk()
    {
        std::unique_lock lock{mut};
        assert(tail_ != nullptr);

        if (size_ == 0)
        {

        }
    }

};