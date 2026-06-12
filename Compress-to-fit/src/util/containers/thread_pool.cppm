export module containers:thread_pool;
import std.compat;
#include <type_traits>


/**
 * @brief A tool to asynchronously execute tasks without respawning threads
 */
export class ThreadPool
{
public:
    /**
     * @param n_threads The number of threads to spawn. Defaults to the provided thread count from the OS.
     */
    ThreadPool(size_t n_threads = std::thread::hardware_concurrency())
    {
        if (n_threads == 0) n_threads = 1;

        threads_.reserve(n_threads);
        for (size_t i = 0; i < n_threads; i++)
            threads_.emplace_back(&worker, this);
    }

    ~ThreadPool()
    {
        {
            std::unique_lock lock{mut_};
            stop_threads_ = true;
        }

        cv_.notify_all();//notify threads
    }

    //when in doubt:
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool(ThreadPool const&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool const&) = delete;

    /**
     * @brief Add a new task for the thread pool
     * @tparam F Function type
     * @tparam Args Argument types
     * @param f function
     * @param args arguments
     * @return The future of the task
     */
    template <typename F, typename... Args>
    auto add_task(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
    {
        //make a callable with          function v             arguments v...
        auto command = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        //make a shared_ptr to a task for the callable
        auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(command);

        //get the future before we start
        std::future<std::result_of_t<F(Args...)>> future = task->get_future();

        {
            std::unique_lock lock{mut_};
            //enqueue the command
            tasks_.emplace([task]
                {
                    (*task)();
                });
        }
        //execute the command
        cv_.notify_one();
        return future;
    }

private:
    std::vector<std::jthread> threads_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex mut_;
    std::condition_variable cv_;
    bool stop_threads_{false};


    /**
     * @brief The callable threads execute.
     */
    void worker()
    {
        for (;;)
        {
            std::function<void()> cur_task;

            {
                std::unique_lock lock{mut_};

                //after being notified, check that stop is true or that the queue is not empty (if stop_threads_ is true, we go to the next and destroy this thread)
                cv_.wait(lock, [this]
                    {
                        return stop_threads_ or !tasks_.empty();
                    });

                if (stop_threads_ and tasks_.empty())
                    break;
                if (tasks_.empty())
                    continue;

                cur_task = tasks_.front();
                tasks_.pop();
            }

            cur_task();
        }
    }

};
