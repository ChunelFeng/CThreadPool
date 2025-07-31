/***************************
@Author: Chunel
@Contact: chunel@foxmail.com
@File: UThreadPool.h
@Time: 2021/7/4 1:34 下午
@Desc: 
***************************/

#ifndef CGRAPH_UTHREADPOOL_H
#define CGRAPH_UTHREADPOOL_H

#include <vector>
#include <list>
#include <map>
#include <future>
#include <thread>
#include <algorithm>
#include <memory>
#include <functional>

#include "UThreadObject.h"
#include "UThreadPoolConfig.h"
#include "Queue/UQueueInclude.h"
#include "Thread/UThreadInclude.h"
#include "Task/UTaskInclude.h"

CGRAPH_NAMESPACE_BEGIN

class UThreadPool : public UThreadObject {
public:
    /**
     * 通过默认设置参数，来创建线程池
     * @param autoInit 是否自动开启线程池功能
     * @param config
     */
    explicit UThreadPool(CBool autoInit = true,
                         const UThreadPoolConfig& config = UThreadPoolConfig()) noexcept {
        cur_index_ = 0;
        is_init_ = false;
        this->setConfig(config);    // setConfig 函数，用在 is_init_ 设定之后
        if (autoInit) {
            this->init();
        }
    }

    /**
     * 析构函数
     */
    ~UThreadPool() override {
        this->config_.monitor_enable_ = false;    // 在析构的时候，才释放监控线程。先释放监控线程，再释放其他的线程
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }

        destroy();
    }

    /**
     * 设置线程池相关配置信息，需要在init()函数调用前，完成设置
     * @param config
     * @return
     * @notice 通过单例类(UThreadPoolSingleton)开启线程池，则线程池默认init。需要 destroy 后才可以设置参数
     */
    CStatus setConfig(const UThreadPoolConfig &config) {
        CGRAPH_FUNCTION_BEGIN
        CGRAPH_ASSERT_INIT(false)    // 初始化后，无法设置参数信息

        this->config_ = config;
        CGRAPH_FUNCTION_END
    }

    /**
     * 获取线程池配置信息
     * @return
     */
    UThreadPoolConfig getConfig() const {
        return config_;
    }

    /**
     * 开启所有的线程信息
     * @return
     */
    CStatus init() final {
        CGRAPH_FUNCTION_BEGIN
        if (is_init_) {
            CGRAPH_FUNCTION_END
        }

        if (config_.monitor_enable_) {
            // 默认不开启监控线程
            monitor_thread_ = std::thread(&UThreadPool::monitor, this);
        }
        thread_record_map_.clear();
        thread_record_map_[(CSize)std::hash<std::thread::id>{}(std::this_thread::get_id())] = CGRAPH_MAIN_THREAD_ID;
        task_queue_.setup();
        primary_threads_.reserve(config_.default_thread_size_);
        for (int i = 0; i < config_.default_thread_size_; i++) {
            auto* pt = CGRAPH_SAFE_MALLOC_COBJECT(UThreadPrimary);    // 创建核心线程数
            pt->setThreadPoolInfo(i, &task_queue_, &primary_threads_, &config_);
            // 记录线程和匹配id信息
            primary_threads_.emplace_back(pt);
        }

        /**
         * 等待所有thread 设置完毕之后，再进行 init()，
         * 避免在个别的平台上，可能出现 thread竞争访问其他线程、并且导致异常的情况
         * 参考： https://github.com/ChunelFeng/CGraph/issues/309
         */
        for (int i = 0; i < config_.default_thread_size_; i++) {
            status += primary_threads_[i]->init();
            thread_record_map_[(CSize)std::hash<std::thread::id>{}(primary_threads_[i]->thread_.get_id())] = i;
        }
        CGRAPH_FUNCTION_CHECK_STATUS

        /**
         * 策略更新：
         * 初始化的时候，也可以创建n个辅助线程。目的是为了配合仅使用 pool中 priority_queue 的场景
         * 一般情况下，建议为0。
         */
        status = createSecondaryThread(config_.secondary_thread_size_);
        CGRAPH_FUNCTION_CHECK_STATUS

        is_init_ = true;
        CGRAPH_FUNCTION_END
    }

    /**
     * 提交任务信息
     * @tparam FunctionType
     * @param func
     * @param index
     * @return
     */
    template<typename FunctionType>
    auto commit(const FunctionType& func,
                CIndex index = CGRAPH_DEFAULT_TASK_STRATEGY)
    -> std::future<decltype(std::declval<FunctionType>()())>;

    /**
     * 向特定的线程id中，提交任务信息
     * @tparam FunctionType
     * @param func
     * @param tid 线程id。如果超出主线程个数范围，则默认写入pool的通用队列中
     * @param enable 是否启用上锁/解锁功能
     * @param lockable 上锁(true) / 解锁(false)
     * @return
     */
    template<typename FunctionType>
    auto commitWithTid(const FunctionType& func, CIndex tid, CBool enable, CBool lockable)
    -> std::future<decltype(std::declval<FunctionType>()())>;

    /**
     * 根据优先级，执行任务
     * @tparam FunctionType
     * @param func
     * @param priority 优先级别。自然序从大到小依次执行
     * @return
     * @notice 建议，priority 范围在 [-100, 100] 之间
     */
    template<typename FunctionType>
    auto commitWithPriority(const FunctionType& func,
                            int priority)
    -> std::future<decltype(std::declval<FunctionType>()())>;

    /**
     * 异步执行任务
     * @tparam FunctionType
     * @param task
     * @param index
     */
    template<typename FunctionType>
    CVoid execute(FunctionType&& task,
                  CIndex index = CGRAPH_DEFAULT_TASK_STRATEGY);

    /**
     * 异步写入特定thread id，执行信息
     * @tparam FunctionType
     * @param task
     * @param tid
     * @param enable
     * @param lockable
     * @return
     */
    template<typename FunctionType>
    CVoid executeWithTid(FunctionType&& task, CIndex tid, CBool enable, CBool lockable);

    /**
     * 执行任务组信息
     * 取taskGroup内部ttl和入参ttl的最小值，为计算ttl标准
     * @param taskGroup
     * @param ttl
     * @return
     */
    CStatus submit(const UTaskGroup& taskGroup,
                   CMSec ttl = CGRAPH_MAX_BLOCK_TTL) {
        CGRAPH_FUNCTION_BEGIN
        CGRAPH_ASSERT_INIT(true)

        std::vector<std::future<CVoid>> futures;
        futures.reserve(taskGroup.getSize());
        for (const auto& task : taskGroup.task_arr_) {
            futures.emplace_back(commit(task));
        }

        // 计算最终运行时间信息
        auto deadline = std::chrono::steady_clock::now()
                        + std::chrono::milliseconds(std::min(taskGroup.getTtl(), ttl));

        for (auto& fut : futures) {
            const auto& futStatus = fut.wait_until(deadline);
            switch (futStatus) {
                case std::future_status::ready: break;    // 正常情况，直接返回了
                case std::future_status::timeout: status += CStatus("thread status timeout"); break;
                case std::future_status::deferred: status += CStatus("thread status deferred"); break;
                default: status += CStatus("thread status unknown");
            }
        }

        if (taskGroup.on_finished_) {
            taskGroup.on_finished_(status);
        }

        CGRAPH_FUNCTION_END
    }

    /**
     * 针对单个任务的情况，复用任务组信息，实现单个任务直接执行
     * @param task
     * @param ttl
     * @param onFinished
     * @return
     */
    CStatus submit(CGRAPH_DEFAULT_CONST_FUNCTION_REF func,
                   CMSec ttl = CGRAPH_MAX_BLOCK_TTL,
                   CGRAPH_CALLBACK_CONST_FUNCTION_REF onFinished = nullptr) {
        return submit(UTaskGroup(func, ttl, onFinished));
    }

    /**
     * 获取根据线程id信息，获取线程index信息
     * @param tid
     * @return
     * @notice 辅助线程返回-1
     */
    CIndex getThreadIndex(CSize tid) {
        int index = CGRAPH_SECONDARY_THREAD_COMMON_ID;
        auto result = thread_record_map_.find(tid);
        if (result != thread_record_map_.end()) {
            index = result->second;
        }

        return index;
    }

    /**
     * 释放所有的线程信息
     * @return
     */
    CStatus destroy() final {
        CGRAPH_FUNCTION_BEGIN
        if (!is_init_) {
            CGRAPH_FUNCTION_END
        }

        // primary 线程是普通指针，需要delete
        for (auto &pt : primary_threads_) {
            status += pt->destroy();
        }
        CGRAPH_FUNCTION_CHECK_STATUS

        /**
         * 这里之所以 destroy和 delete分开两个循环执行，
         * 是因为当前线程被delete后，还可能存在未被delete的主线程，来steal当前线程的任务
         * 在windows环境下，可能出现问题。
         * destroy 和 delete 分开之后，不会出现此问题。
         * 感谢 Ryan大佬(https://github.com/ryanhuang) 提供的帮助
         */
        for (auto &pt : primary_threads_) {
            CGRAPH_DELETE_PTR(pt)
        }
        primary_threads_.clear();

        // secondary 线程是智能指针，不需要delete
        task_queue_.reset();
        for (auto &st : secondary_threads_) {
            status += st->destroy();
        }
        CGRAPH_FUNCTION_CHECK_STATUS
        secondary_threads_.clear();
        thread_record_map_.clear();
        is_init_ = false;

        CGRAPH_FUNCTION_END
    }

    /**
     * 判断线程池是否已经初始化了
     * @return
     */
    CBool isInit() const {
        return is_init_;
    }

    /**
     * 生成辅助线程。内部确保辅助线程数量不超过设定参数
     * @param size
     * @return
     */
    CStatus createSecondaryThread(CInt size) {
        CGRAPH_FUNCTION_BEGIN

        int leftSize = (int)(config_.max_thread_size_ - config_.default_thread_size_ - secondary_threads_.size());
        int realSize = std::min(size, leftSize);    // 使用 realSize 来确保所有的线程数量之和，不会超过设定max值

        CGRAPH_LOCK_GUARD lock(st_mutex_);
        for (int i = 0; i < realSize; i++) {
            auto ptr = CGRAPH_MAKE_UNIQUE_COBJECT(UThreadSecondary)
            ptr->setThreadPoolInfo(&task_queue_, &priority_task_queue_, &config_);
            status += ptr->init();
            secondary_threads_.emplace_back(std::move(ptr));
        }

        CGRAPH_FUNCTION_END
    }

    /**
     * 删除辅助线程
     * @param size
     * @return
     */
    CStatus releaseSecondaryThread(CInt size) {
        CGRAPH_FUNCTION_BEGIN

        // 先将所有已经结束的，给删掉
        CGRAPH_LOCK_GUARD lock(st_mutex_);
        for (auto iter = secondary_threads_.begin(); iter != secondary_threads_.end(); ) {
            !(*iter)->done_ ? secondary_threads_.erase(iter++) : iter++;
        }

        CGRAPH_RETURN_ERROR_STATUS_BY_CONDITION((size > (CInt)secondary_threads_.size()),    \
                                            "cannot release [" + std::to_string(size) + "] secondary thread,"    \
                                            + "only [" + std::to_string(secondary_threads_.size()) + "] left.")

        // 再标记几个需要删除的信息
        for (auto iter = secondary_threads_.begin();
             iter != secondary_threads_.end() && size-- > 0; ) {
            (*iter)->done_ = false;
            iter++;
        }
        CGRAPH_FUNCTION_END
    }

    /**
     * 通知所有thread 开启
     * @return
     */
    CVoid wakeupAllThread() {
        for (auto& pt : primary_threads_) {
            pt->wakeup();
        }

        for (auto& st : secondary_threads_) {
            st->wakeup();
        }
    }

protected:
    /**
     * 根据传入的策略信息，确定最终执行方式
     * @param origIndex
     * @return
     */
    virtual CIndex dispatch(CIndex origIndex) {
        CIndex realIndex = 0;
        if (CGRAPH_DEFAULT_TASK_STRATEGY == origIndex) {
            realIndex = cur_index_++;
            if (cur_index_ >= config_.max_thread_size_ || cur_index_ < 0) {
                cur_index_ = 0;
            }
        } else {
            realIndex = origIndex;
        }

        return realIndex;    // 交到上游去判断，走哪个线程
    }

    /**
     * 监控线程执行函数，主要是判断是否需要增加线程，或销毁线程
     * 增/删 操作，仅针对secondary类型线程生效
     */
    CVoid monitor() {
        while (config_.monitor_enable_) {
            while (config_.monitor_enable_ && !is_init_) {
                // 如果没有init，则一直处于空跑状态
                CGRAPH_SLEEP_SECOND(1)
            }

            auto span = config_.monitor_span_;
            while (config_.monitor_enable_ && is_init_ && span--) {
                CGRAPH_SLEEP_SECOND(1)    // 保证可以快速退出
            }

            // 如果 primary线程都在执行，则表示忙碌
            bool busy = !primary_threads_.empty() && std::all_of(primary_threads_.begin(), primary_threads_.end(),
                                                                 [](UThreadPrimaryPtr ptr) { return ptr && ptr->is_running_; });

            CGRAPH_LOCK_GUARD lock(st_mutex_);
            // 如果忙碌或者priority_task_queue_中有任务，则需要添加 secondary线程
            if (busy || !priority_task_queue_.empty()) {
                createSecondaryThread(1);
            }

            // 判断 secondary 线程是否需要退出
            for (auto iter = secondary_threads_.begin(); iter != secondary_threads_.end(); ) {
                (*iter)->freeze() ? secondary_threads_.erase(iter++) : iter++;
            }
        }
    }

    CGRAPH_NO_ALLOWED_COPY(UThreadPool)

private:
    CBool is_init_ { false };                                                       // 是否初始化
    CInt cur_index_ = 0;                                                            // 记录放入的线程数
    UAtomicQueue<UTask> task_queue_;                                                // 用于存放普通任务
    UAtomicPriorityQueue<UTask> priority_task_queue_;                               // 运行时间较长的任务队列，仅在辅助线程中执行
    std::vector<UThreadPrimaryPtr> primary_threads_;                                // 记录所有的主线程
    std::list<std::unique_ptr<UThreadSecondary>> secondary_threads_;                // 用于记录所有的辅助线程
    UThreadPoolConfig config_;                                                      // 线程池设置值
    std::thread monitor_thread_;                                                    // 监控线程
    std::map<CSize, int> thread_record_map_;                                        // 线程记录的信息
    std::mutex st_mutex_;                                                           // 辅助线程发生变动的时候，加的mutex信息
};

using UThreadPoolPtr = UThreadPool *;

CGRAPH_NAMESPACE_END

#include "UThreadPool.inl"

#endif //CGRAPH_UTHREADPOOL_H
