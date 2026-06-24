#pragma once

#include <vector>
#include <functional>
#include <atomic>
#include <string>

#include "Job.h"

namespace ego
{
    class JobController;

    class JobGraph;
    EGO_POINTER(JobGraph);
    using JobGraphReference = JobGraphPointer;

    enum class JobGraphState
    {
        Undefined,
        Pending,
        Finished
    };

    class JobGraph final : public STDDestroyMTCountable
    {
    public:
        class GraphJobData final : public STDDestroyMTCountable
        {
            friend class JobGraphBuilder;
            friend class JobGraph;

        public:
            using CompletionCallback = std::function<void()>;
            using CompletionCallbackCollection = std::vector<CompletionCallback>;

        public:
            GraphJobData(const char* _dbgName = nullptr);
            ~GraphJobData() override = default;

            JobControllerPointer getJobController() const
            {
                return m_jobController.lock();
            }
            void completeJob();

            void addCompletionCallback(const CompletionCallback& _callback);

            void setExecutionContext(const JobControllerWeakPointer& _controller);

            void wait();
            bool isFinished() const;
            JobGraphState getState() const
            {
                return m_state;
            }

#ifdef EGO_JOB_DEBUG
            const char* getDbgName() const
            {
                return m_dbgName.c_str();
            }
            const char* getBeginBarrierDbgName() const
            {
                return m_beginBarrierDbgName.c_str();
            }
            const char* getEndBarrierDbgName() const
            {
                return m_endBarrierDbgName.c_str();
            }
#endif

        private:
            CompletionCallbackCollection m_completionCallback;
            std::atomic<uint32_t> m_jobCounter = 0;
            std::atomic<JobGraphState> m_state = JobGraphState::Undefined;
            JobControllerWeakPointer m_jobController;

            std::condition_variable m_completionNotifier;
            std::mutex m_mutex;

#ifdef EGO_JOB_DEBUG
            std::string m_dbgName;
            std::string m_beginBarrierDbgName;
            std::string m_endBarrierDbgName;
#endif
        };

        using GraphJobDataReference = SharedPointer<GraphJobData>;

        class DependencyJob;
        using DependencyJobReference = SharedPointer<DependencyJob>;
        using DependencyJobCollection = std::vector<DependencyJobReference>;
        using JobGraphCollection = std::vector<JobGraphReference>;

        class DependencyJob : public Job
        {
            friend class JobGraphBuilder;
            friend class JobGraph;

        public:
            using DependencyCollection = std::vector<DependencyJobReference>;

            bool removeParentDependency();

        protected:
            DependencyJob(const GraphJobDataReference& _graphData, const char* _dbgName);

        private:
            virtual bool executeDependencyJob() = 0;

            void removeChildDependencies();

            void operate() final;

            DependencyCollection m_dependencyJobs;

            GraphJobDataReference m_graphData;
            std::atomic<uint32_t> m_parentCounter = 0;
        };

        class JobDependencyJob final : public DependencyJob
        {
        public:
            JobDependencyJob(const JobReference& _job, const GraphJobDataReference& _graphData);

        private:
            bool executeDependencyJob() override;

            JobReference m_job;
        };

        class BarrierDependencyJob final : public DependencyJob
        {
        public:
            enum class Type
            {
                Begin,
                End
            };

            BarrierDependencyJob(const GraphJobDataReference& _graphData, Type _type);

        private:
            bool executeDependencyJob() override;

            Type m_type = Type::Begin;
        };

        using JobCollection = std::vector<JobReference>;

        JobGraph(const DependencyJobReference& _entryJob, const DependencyJobReference& _exitJob, JobGraphCollection&& _nestedGraphs, GraphJobDataReference _data);

        GraphJobDataReference getGraphData() const;

        void setExecutionContext(const JobControllerWeakPointer& _jobController);

        void wait();
        bool isFinished() const;

#ifdef EGO_JOB_DEBUG
        const char* getDbgName() const;
#endif

    private:
        friend class JobController;
        friend class JobGraphBuilder;

        bool trySetExecutionContext(const JobControllerWeakPointer& _jobController);
        bool schedule(const JobControllerWeakPointer& _jobController, JobCollection& _baseJobs);
        bool isScheduled() const;

        DependencyJobReference m_entryJob;
        DependencyJobReference m_exitJob;
        JobGraphCollection m_nestedGraphs;
        GraphJobDataReference m_graphData;
    };

    class JobGraphBuilder final
    {
    private:
        struct GraphBuildingContext;

    public:
        struct JobGraphJobID final
        {
            static constexpr uint32_t InvalidIndex = 0xFFFFFFFFu;

            JobGraphJobID() = default;

            bool isValid() const;
            uint32_t getIndex() const;

            bool operator==(JobGraphJobID _id) const;
            bool operator!=(JobGraphJobID _id) const;

        private:
            friend class JobGraphBuilder;
            friend struct GraphBuildingContext;

            JobGraphJobID(const void* _owner, uint32_t _generation, uint32_t _index);

            const void* m_owner = nullptr;
            uint32_t m_generation = 0;
            uint32_t m_index = InvalidIndex;
        };

        using JobGraphJobIDCollection = std::vector<JobGraphJobID>;

        explicit JobGraphBuilder(const char* _dbgName = nullptr);

        void setDbgName(const char* _dbgName);
        const char* getDbgName() const
        {
            return m_dbgName.c_str();
        }

        JobGraphJobID addJob(const JobReference& _job);
        JobGraphJobID addJobBefore(const JobReference& _job, JobGraphJobID _childJobID);
        JobGraphJobID addJobBefore(const JobReference& _job, const JobGraphJobIDCollection& _childJobIDs);
        JobGraphJobID addJobAfter(const JobReference& _job, JobGraphJobID _parentJobID);
        JobGraphJobID addJobAfter(const JobReference& _job, const JobGraphJobIDCollection& _parentJobIDs);
        JobGraphJobID addJobBetween(const JobReference& _job, JobGraphJobID _parentJobID, JobGraphJobID _childJobID);
        JobGraphJobID addJobBetween(const JobReference& _job, const JobGraphJobIDCollection& _parentJobIDs, const JobGraphJobIDCollection& _childJobIDs);

        JobGraphJobID addJobGraph(const JobGraphReference& _graph);
        JobGraphJobID addJobGraphBefore(const JobGraphReference& _graph, JobGraphJobID _childJobID);
        JobGraphJobID addJobGraphBefore(const JobGraphReference& _graph, const JobGraphJobIDCollection& _childJobIDs);
        JobGraphJobID addJobGraphAfter(const JobGraphReference& _graph, JobGraphJobID _parentJobID);
        JobGraphJobID addJobGraphAfter(const JobGraphReference& _graph, const JobGraphJobIDCollection& _parentJobIDs);
        JobGraphJobID addJobGraphBetween(const JobGraphReference& _graph, JobGraphJobID _parentJobID, JobGraphJobID _childJobID);
        JobGraphJobID addJobGraphBetween(const JobGraphReference& _graph, const JobGraphJobIDCollection& _parentJobIDs, const JobGraphJobIDCollection& _childJobIDs);

        void makeDependency(JobGraphJobID _parentJobID, JobGraphJobID _childJobID);

        JobGraphReference getGraph();

        static constexpr uint32_t InvalidJobGraphJobIndex = JobGraphJobID::InvalidIndex;

    private:
        struct GraphBuildingContext final
        {
            enum class NodeType
            {
                Job,
                JobGraph
            };

            struct Node final
            {
                NodeType m_type = NodeType::Job;
                JobReference m_job;
                JobGraphReference m_jobGraph;
            };

            struct Dependency final
            {
                JobGraphJobID m_parentJobID;
                JobGraphJobID m_childJobID;
            };

            GraphBuildingContext();
            void clear();

            JobGraphJobID addJob(const JobReference& _job);
            JobGraphJobID addJobGraph(const JobGraphReference& _jobGraph);
            bool isValid(JobGraphJobID _jobID) const;

            std::vector<Node> m_nodes;
            std::vector<Dependency> m_dependencies;
            uint32_t m_generation = 0;
        };

        void makeDependenciesBefore(JobGraphJobID _parentJobID, const JobGraphJobIDCollection& _childJobIDs);
        void makeDependenciesAfter(const JobGraphJobIDCollection& _parentJobIDs, JobGraphJobID _childJobID);

        GraphBuildingContext m_buildingContext;
        std::string m_dbgName;
    };
} // namespace ego
