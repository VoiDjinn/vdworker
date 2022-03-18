#include "VDWorker.h"

/*
******************************
********** VDWorker **********
******************************
*/

Ref<VDWorker> VDWorker::singleton;

VDWorker::VDWorker() {}

void VDWorker::_bind_methods()
{
    ClassDB::bind_method ( D_METHOD ( "execute", "job" ), &VDWorker::execute );
    ClassDB::bind_method ( D_METHOD ( "_job_thread_exit", "job" ), &VDWorker::_job_thread_exit );
    ClassDB::bind_method ( D_METHOD ( "_handle_job_execution_finished", "job" ), &VDWorker::_handle_job_execution_finished );

    ADD_SIGNAL ( MethodInfo ( "started" ) );
    ADD_SIGNAL ( MethodInfo ( "finished" ) );
    ADD_SIGNAL ( MethodInfo ( "job_started", PropertyInfo ( Variant::OBJECT, "job" ) ) );
    ADD_SIGNAL ( MethodInfo ( "job_finished", PropertyInfo ( Variant::OBJECT, "job" ) ) );
}

void VDWorker::register_job ( Ref<VDJob> job )
{
    if ( !is_job_registered ( job ) ) {
        job->worker = Ref<VDWorker> ( this );
        registered_jobs.insert ( job );
    }
}

void VDWorker::unregister_job ( Ref<VDJob> job )
{
    if ( is_job_registered ( job ) ) {
        job->worker.unref();
        registered_jobs.erase ( job );
    }
}

bool VDWorker::is_job_registered ( Ref<VDJob> job )
{
    return job->worker == this;
}

void VDWorker::execute ( Ref<VDJob> job )
{
    if ( job->worker.is_null() ) {
        job->worker = Ref<VDWorker> ( this );
        registered_jobs.insert ( job );
        connect ( "job_finished", this, "_handle_job_execution_finished", Vector<Variant>(),CONNECT_ONESHOT );
        job->start();
    } else if ( job->worker == this && job->current_state == VDJob::PAUSED ) {
        job->start();
    }
}

void VDWorker::_handle_job_execution_finished ( Ref<VDJob> job )
{
    if ( job->worker.is_valid() ) job->worker.unref();
    registered_jobs.erase ( job );
}

void VDWorker::_job_thread_start_func ( void *p_ud )
{
    VDJob * job = ( VDJob * ) p_ud;
    if ( job->worker->active_jobs.size() == 0 )
        job->worker->emit_signal ( "started" );
    job->worker->emit_signal ( "job_started", job );
    job->emit_signal ( "started" );
    job->currentIndex = 0;
    _job_thread_process_func ( p_ud );
}

void VDWorker::_job_thread_process_func ( void *p_ud )
{
    Ref<VDJob> job = Ref<VDJob> ( ( VDJob * ) p_ud );
    bool restarted = job->current_state == VDJob::PAUSED;
    job->set_current_state ( VDJob::ON );
    job->worker->active_jobs.insert ( job );
    Ref<VDTask> currentTask;
    while ( ( job->currentIndex < job->tasks.size() ) && job->current_state == VDJob::ON ) {
        currentTask = job->tasks[job->currentIndex];
        if ( !restarted ) {
            job->emit_signal ( "task_started", currentTask );
            currentTask->emit_signal ( "started" );
            currentTask->init();
        }
        while ( !currentTask->execute() ) {
            if ( job->current_state != VDJob::ON ) {
                job->worker->call_deferred ( "_job_thread_exit", job );
                return;
            }
        }
        currentTask->finish();
        currentTask->emit_signal ( "finished" );
        job->emit_signal ( "task_finished", currentTask );
        job->currentIndex += 1;
        restarted = false;
    }
    job->worker->call_deferred ( "_job_thread_exit", job );
}

void VDWorker::_job_thread_exit ( Ref<VDJob> job )
{
    Thread *job_thread = job->process_thread;
    job_thread->wait_to_finish();
    //Thread::wait_to_finish ( job_thread );
    memdelete ( job_thread );
    job->process_thread = NULL;
    this->active_jobs.erase ( job );
    if ( job->current_state != VDJob::PAUSE_INITIATED ) {
        Ref<VDWorker> worker = job->worker;
        if ( job->current_state == VDJob::OFF_INITIATED ) job->set_current_state ( VDJob::OFF );
        else {
            job->emit_signal ( "finished" );
            worker->emit_signal ( "job_finished", job );
            job->set_current_state ( VDJob::DONE );
        }
        if ( this->active_jobs.size() == 0 ) worker->emit_signal ( "finished" );
    } else {
        job->set_current_state ( ( VDJob::PAUSED ) );
    }
}

/*
******************************
********** VDJob **********
******************************
*/

VDJob::VDJob() {}

void VDJob::_bind_methods()
{
    ClassDB::bind_method ( D_METHOD ( "add_task", "task" ), &VDJob::add_task );
    ClassDB::bind_method ( D_METHOD ( "remove_task", "task" ), &VDJob::remove_task );

    ClassDB::bind_method ( D_METHOD ( "_start_after_cleanup", "state" ), &VDJob::_start_after_cleanup );

    //ClassDB::bind_method ( D_METHOD ( "get_current_state"), &VDJob::get_current_state );
    ClassDB::bind_method ( D_METHOD ( "get_progress" ), &VDJob::get_progress );
    ClassDB::bind_method ( D_METHOD ( "get_identifier" ), &VDJob::get_identifier );

    ClassDB::bind_method ( D_METHOD ( "start" ), &VDJob::start );
    ClassDB::bind_method ( D_METHOD ( "pause" ), &VDJob::pause );
    ClassDB::bind_method ( D_METHOD ( "stop" ), &VDJob::stop );

    ADD_SIGNAL ( MethodInfo ( "started" ) );
    ADD_SIGNAL ( MethodInfo ( "finished" ) );
    ADD_SIGNAL ( MethodInfo ( "status_changed", PropertyInfo ( Variant::INT, "status" ) ) );
    ADD_SIGNAL ( MethodInfo ( "task_started", PropertyInfo ( Variant::OBJECT, "task" ) ) );
    ADD_SIGNAL ( MethodInfo ( "task_finished", PropertyInfo ( Variant::OBJECT, "task" ) ) );
    ADD_SIGNAL ( MethodInfo ( "update_progress", PropertyInfo ( Variant::REAL, "progress" ) ) );
}

void VDJob::add_task ( Ref<VDTask> task )
{
    if ( !task->owning_job ) {
        task->owning_job = this;
        this->tasks.push_back ( task );
    }
}

void VDJob::remove_task ( Ref<VDTask> task )
{
    if ( tasks.find ( task ) ) {
        task->owning_job = NULL;
        tasks.erase ( task );
    }
}

void VDJob::update_progress ( float task_progress )
{
    current_progress = ( task_progress + currentIndex ) / tasks.size();
    emit_signal ( "update_progress", current_progress );
}

void VDJob::start()
{
    if ( current_state != ON ) {
        if ( current_state == PAUSE_INITIATED || current_state == OFF_INITIATED ) {
            connect ( "status_changed", this, "_start_after_cleanup", Vector<Variant>(), CONNECT_ONESHOT );
            return;
        } else {
            void ( *ThreadCallback ) ( void *p_result );
            if ( current_state == OFF || current_state == DONE ) {
                ThreadCallback = VDWorker::_job_thread_start_func;
            } else {
                ThreadCallback = VDWorker::_job_thread_process_func;
            }
            process_thread = memnew(Thread); //Thread::create ( ThreadCallback, this );
            process_thread->start(ThreadCallback, this);
        }
    }
}

void VDJob::pause()
{
    if ( current_state == ON ) {
        set_current_state ( PAUSE_INITIATED );
    }
}

void VDJob::stop()
{
    if ( current_state == PAUSED ) {
        set_current_state ( OFF );
    } else if ( current_state != OFF ) {
        set_current_state ( OFF_INITIATED );
    }
}

void VDJob::set_current_state ( VDJob::JobState state )
{
    this->current_state = state;
    emit_signal ( "status_changed", current_state );
}

void VDJob::_start_after_cleanup ( int state )
{
    if ( state == PAUSED || state == OFF ) {
        start();
    }
}

VDJob::JobState VDJob::get_current_state() const
{
    return current_state;
}

float VDJob::get_progress() const
{
    return current_progress;
}

String VDJob::get_identifier() const
{
    return this->identifier;
}

/*
******************************
********** VDTask **********
******************************
*/

VDTask::VDTask()
{
    owning_job = nullptr;
    resultCallback = VDTask::dummyResultCallback;
}

void VDTask::_bind_methods()
{

    ClassDB::bind_method ( D_METHOD ( "get_progress" ), &VDTask::get_progress );
    ClassDB::bind_method ( D_METHOD ( "get_identifier" ), &VDTask::get_identifier );
    ClassDB::bind_method ( D_METHOD ( "get_owning_job" ), &VDTask::get_owning_job );

    ADD_SIGNAL ( MethodInfo ( "started" ) );
    ADD_SIGNAL ( MethodInfo ( "finished" ) );
    ADD_SIGNAL ( MethodInfo ( "update_progress", PropertyInfo ( Variant::REAL, "progress" ) ) );
    ADD_SIGNAL ( MethodInfo ( "result_returned", PropertyInfo ( Variant::NIL, "result" ) ) );
}

void VDTask::update_progress ( float progress )
{
    current_progress = progress;
    emit_signal ( "update_progress", current_progress );
    owning_job->update_progress ( current_progress );
}

void VDTask::call_result_callback ( void* result )
{
    resultCallback ( resultCallbackUserData, result );
}

float VDTask::get_progress() const
{
    return current_progress;
}

String VDTask::get_identifier() const
{
    return this->identifier;
}

VDJob *VDTask::get_owning_job() const
{
    return this->owning_job;
}

void VDTask::set_result_callback ( TaskResultCallback callback, void *resultCallbackUserData )
{
    CRASH_COND_MSG ( resultCallback != VDTask::dummyResultCallback, "Cannot change an already setted result callback." );
    resultCallback = callback;
    this->resultCallbackUserData = resultCallbackUserData;
}

void VDTask::dummyResultCallback ( void *resultUserData, void *result ) {}

