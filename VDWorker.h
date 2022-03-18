#ifndef VDWORKER_H
#define VDWORKER_H

#include "core/set.h"
#include "core/reference.h"
#include "core/os/thread.h"

class VDJob;
class VDTask;

typedef void ( *TaskResultCallback ) ( void *resultCallbackUserData, void *p_result );

class VDWorker : public Reference
{
    GDCLASS ( VDWorker, Reference );

    friend class VDJob;

	Set<Ref<VDJob>> registered_jobs;
    Set<Ref<VDJob>> active_jobs;

    // thread methods
    void _job_thread_exit (Ref<VDJob> job );
    static void _job_thread_start_func ( void *p_ud );
    static void _job_thread_process_func ( void *p_ud );

	void _handle_job_execution_finished(Ref<VDJob> job);

protected:
    static void _bind_methods();

public:
    VDWorker();

    static Ref<VDWorker> singleton;
    static VDWorker *get_singleton()
    {
        return singleton.ptr();
    }

    void register_job(Ref<VDJob> job);
    void unregister_job(Ref<VDJob> job);
	bool is_job_registered(Ref<VDJob> job);
    void execute ( Ref<VDJob> job );
};

class VDJob : public Reference
{
    GDCLASS ( VDJob, Reference );

    enum JobState {
        OFF = 0,
        PAUSED = 1,
        ON = 2,
        PAUSE_INITIATED = 3,
        OFF_INITIATED = 4,
        DONE = 5
    };

    friend class VDWorker;
    friend class VDTask;

    List<Ref<VDTask>> tasks;

    Ref<VDWorker> worker;
    Thread *process_thread;

    void set_current_state ( JobState state );
    void _start_after_cleanup ( int state );

protected:
    static void _bind_methods();

    String identifier;
    int currentIndex = 0;
    float current_progress = 0.0;
    JobState current_state = JobState::OFF;

    void update_progress ( float task_progress );

public:
    VDJob();

    void add_task ( Ref<VDTask> task );
    void remove_task ( Ref<VDTask> task );

    void start();
    void pause();
    void stop();
    JobState get_current_state() const;

    String get_identifier() const;
    float get_progress() const;
};

class VDTask : public Reference
{
    GDCLASS ( VDTask, Reference );

    friend class VDJob;

    VDJob *owning_job;
    float current_progress = 0.0;

    TaskResultCallback resultCallback;
    void * resultCallbackUserData;
    static void dummyResultCallback ( void * resultUserData, void * result );

protected:
    static void _bind_methods();

    String identifier;

    void update_progress ( float progress );
    void call_result_callback ( void* result );

public:
    VDTask();

    String get_identifier() const;
    float get_progress() const;
    VDJob* get_owning_job() const;
    void set_result_callback ( TaskResultCallback callback, void * resultCallbackUserData );

    virtual void init() = 0;
    virtual bool execute() = 0;
    virtual void finish() = 0;
};

#endif
