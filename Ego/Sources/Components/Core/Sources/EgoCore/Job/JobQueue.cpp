#include "JobQueue.h"

void ego::JobQueue::init()
{
    std::lock_guard locker(m_mutex);
    m_isReleased = false;
}

void ego::JobQueue::release()
{

    std::lock_guard locker(m_mutex);
    m_isReleased = true;
    m_queue.clear();

    m_wakeCondition.notify_all();
}

void ego::JobQueue::addJob(const JobReference& job)
{
    std::lock_guard locker(m_mutex);

    if (m_isReleased)
    {
        return;
    }

    m_queue.push_back(job);
    m_wakeCondition.notify_one();
}

ego::JobReference ego::JobQueue::getJob()
{
    std::unique_lock locker(m_mutex);
    m_wakeCondition.wait(
        locker,
        [this]()
        {
            return m_isReleased || !m_queue.empty();
        });

    if (m_isReleased)
    {
        return nullptr;
    }

    JobReference job = m_queue.front();
    m_queue.pop_front();

    return job;
}

ego::JobReference ego::JobQueue::tryGetJob()
{
    std::unique_lock locker(m_mutex);

    if (m_isReleased || m_queue.empty())
    {
        return nullptr;
    }

    JobReference job = m_queue.front();
    m_queue.pop_front();

    return job;
}
