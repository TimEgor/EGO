#pragma once

#include <vector>
#include <functional>

#include "Job.h"

namespace ego
{
    //TODO: JobGraph realization is overcomplicated. Should be reworked

    class JobController;

    class JobGraph;
    EGO_REFERENCE(JobGraph);

    class JobGraph final : public STDDestroyMTCountable
    {
    public:
        class GraphJobData final : public STDDestroyMTCountable
        {
            struct JobGraphEvent final
            {
                JobGraphEvent(GraphJobData* _graph);

                void wait();
                void set();

                std::condition_variable m_notifier;
                GraphJobData* m_graph = nullptr;
            };

            friend JobGraphEvent;

            friend class JobGraphBuilder;

        public:
            using CompletionCallback = std::function<void()>;
            using CompletionCallbackCollection = std::vector<CompletionCallback>;

        private:
            CompletionCallbackCollection m_completionCallback;
            std::atomic<uint32_t> m_jobCounter;
            JobControllerWeakPointer m_jobController;
            JobGraphEvent* m_completionEvent = nullptr;

            std::mutex m_mutex;

        public:
            GraphJobData() = default;
            ~GraphJobData();

            JobControllerPointer getJobController() const { return m_jobController.lock(); }
            void completeJob();

            void addCompletionCallback(const CompletionCallback& _callback);

            void setExecutionContext(const JobControllerWeakPointer& _controller);

            void wait();
            bool isFinished() const;
        };

        EGO_REFERENCE(GraphJobData);

        class DependencyJobBase;
        using DependencyJobReference = SharedPointer<DependencyJobBase>;

        class DependencyJobBase : public Job
        {
            friend class JobGraphBuilder;

        public:
            using DependencyCollection = std::vector<DependencyJobReference>;

            DependencyJobBase(const GraphJobDataReference& _graphData, const char* _dbgName);

            bool removeParentDependency();

        protected:
            void removeChildDependencies();

            virtual void operate() override;

            DependencyCollection m_dependencyJobs;

            GraphJobDataReference m_graphData;
            std::atomic<uint32_t> m_parentCounter;

        };

        class DependencyJob final : public DependencyJobBase
        {
        public:
            DependencyJob(const JobReference& _job, const GraphJobDataReference& _graphData);

        private:
            virtual void operate() override;

            JobReference m_job;
        };

        class PostGraphDependencyJob final : public DependencyJobBase
        {
        public:
            PostGraphDependencyJob(const GraphJobDataReference& _graphData);
        };

        using PostGraphDependencyJobReference = SharedPointer<PostGraphDependencyJob>;

        class PreGraphDependencyJob final : public DependencyJobBase
        {
        public:
            PreGraphDependencyJob(
                const GraphJobDataReference& _graphData,
                const PostGraphDependencyJobReference& _postGraphJob,
                const JobGraphReference& _graph
            );

        private:
            virtual void operate() override;

            JobGraphReference m_graph;
        };

        using JobCollection = std::vector<JobReference>;

        JobGraph(JobCollection&& _baseJobs, GraphJobDataReference _data);

        JobCollection&& getBaseJobs();
        GraphJobDataReference getGraphData() const;

        void setExecutionContext(const JobControllerWeakPointer& _jobController);

        void wait();
        bool isFinished() const;

    private:
        JobCollection m_baseJobs;
        GraphJobDataReference m_graphData;
    };

    class JobGraphBuilder final
    {
    public:
        struct JobGraphJobID final
        {
            union
            {
                struct
                {
                    uint32_t m_isGraphJob : 1;
                    uint32_t m_index : 31;
                };

                uint32_t m_value;
            };

            JobGraphJobID(uint32_t _id);
            JobGraphJobID(bool _isGraph, uint32_t _index);

            bool operator==(JobGraphJobID _id) const;

            operator uint32_t() const;
        };

        JobGraphBuilder() = default;
        ~JobGraphBuilder();

        JobGraphJobID addJob(const JobReference& _job);
        JobGraphJobID addJobBefore(const JobReference& _job, JobGraphJobID _childJobID);
        JobGraphJobID addJobAfter(const JobReference& _job, JobGraphJobID _parentJobID);

        JobGraphJobID addJobGraph(const JobGraphReference& _graph);
        JobGraphJobID addJobGraphBefore(const JobGraphReference& _graph, JobGraphJobID _childJobID);
        JobGraphJobID addJobGraphAfter(const JobGraphReference& _graph, JobGraphJobID _parentJobID);

        void makeDependency(JobGraphJobID _parentJobID, JobGraphJobID _childJobID);

        JobGraphReference getGraph();

        static constexpr uint32_t InvalidJobGraphJobIndex = 1u << 30;

    private:
        struct GraphBuildingContext final
        {
            struct GraphNode final
            {
                JobGraphJobID m_preJobID = JobGraphJobID(false, InvalidJobGraphJobIndex);
                JobGraphJobID m_postJobID = JobGraphJobID(false, InvalidJobGraphJobIndex);
            };

            struct GraphJobInfo final
            {
                JobGraph::DependencyJobReference* m_job = nullptr;
                JobGraphJobID m_jobID = JobGraphJobID(false, InvalidJobGraphJobIndex);
            };

            GraphBuildingContext();

            JobGraph::DependencyJobReference* getJob(JobGraphJobID _jobID);
            GraphJobInfo getGraphJob(JobGraphJobID _jobID, bool _isParent);

            JobGraphJobID addDependencyJob(const JobGraph::DependencyJobReference& _job, bool _addToCollection = true);
            JobGraphJobID addGraphNode(JobGraphJobID _preJobID, JobGraphJobID _postJobID);

            std::vector<JobGraph::DependencyJobReference> m_jobs;
            std::vector<GraphNode> m_graphNodes;

            std::vector<JobGraphJobID> m_baseJobIDs;

            std::unordered_map<uint32_t, uint32_t> m_dependencyCounterCollection;

            JobGraph::GraphJobDataReference m_graphData;

            uint32_t m_lastJobIndex = 0;
            uint32_t m_lastGraphIndex = 0;
        };

        void initContext();
        void releaseContext();

        GraphBuildingContext* m_buildingContext = nullptr;
    };
}
