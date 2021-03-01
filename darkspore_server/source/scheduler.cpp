
// Include
#include "scheduler.h"

// Scheduler
decltype(Scheduler::sInstance) Scheduler::sInstance;

Scheduler::Scheduler() {
	mThread = std::thread([this] {
		std::unique_lock<std::mutex> eventLockUnique(mLock, std::defer_lock);
		while (mState.load(std::memory_order_relaxed) == 0) {
			std::cv_status ret = std::cv_status::no_timeout;

			eventLockUnique.lock();
			if (mTasks.empty()) {
				mSignal.wait(eventLockUnique);
			} else {
				ret = mSignal.wait_until(eventLockUnique, mTasks.top()->GetExecutionPoint());
			}

			if (ret != std::cv_status::timeout) {
				eventLockUnique.unlock();
				continue;
			}

			auto task = mTasks.top();
			mTasks.pop();

			auto it = mTaskIds.find(task->GetId());
			if (it == mTaskIds.end()) {
				eventLockUnique.unlock();
				delete task;
				continue;
			}

			mTaskIds.erase(it);
			eventLockUnique.unlock();

			(*task)();
		}
	});
}

uint32_t Scheduler::AddTask(uint32_t delay, const std::function<void(uint32_t)>& f) { return sInstance._AddTask(delay, f); }
void Scheduler::CancelTask(uint32_t id) { sInstance._CancelTask(id); }
void Scheduler::Shutdown() { sInstance._Shutdown(); }

uint32_t Scheduler::_AddTask(uint32_t delay, const std::function<void(uint32_t)>& f) {
	bool do_signal = false;

	mLock.lock();
	if (mState.load(std::memory_order_relaxed) != 0) {
		mLock.unlock();
		return 0;
	}

	if (++mLastTaskId == 0) {
		mLastTaskId = 1;
	}

	auto task = new Task(delay, f);
	task->SetId(mLastTaskId);

	mTaskIds.insert(task->GetId());
	mTasks.push(task);

	do_signal = task == mTasks.top();

	mLock.unlock();
	if (do_signal) {
		mSignal.notify_one();
	}

	return task->GetId();
}

void Scheduler::_CancelTask(uint32_t id) {
	if (id == 0) {
		return;
	}

	std::lock_guard<std::mutex> lockGuard(mLock);
	if (auto it = mTaskIds.find(id); it != mTaskIds.end()) {
		mTaskIds.erase(it);
	}
}

void Scheduler::_Shutdown() {
	mState.store(1, std::memory_order_relaxed);
	mLock.lock();

	while (!mTasks.empty()) {
		delete mTasks.top();
		mTasks.pop();
	}

	mTaskIds.clear();
	mLock.unlock();
	mSignal.notify_one();

	if (mThread.joinable()) {
		mThread.join();
	}
}
