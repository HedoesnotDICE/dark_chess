#include "global_serv.h"
#include <signal.h>
#include <assert.h>
#include "conf_base.h"
#include "abs_conf.h"
#include "proc_state_post.h"

#define SERV_LOG_LEVEL_CHANGE_FLAG ("../conf/serv_log_change.flag")

// 0: 初始状态
// -1: stop
int g_flag = 0;

void sys_sig_kill(int signal)
{
    g_flag = -1;
}

void signal_kill(int signal)
{
    g_flag = -1;
    printf("recv kill signal[%d]\n", signal);
}

// 定义用户信号
TVOID InitSignal()
{
    struct sigaction sa;
    sigset_t sset;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sys_sig_kill;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);

    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    sigemptyset(&sset);
    sigaddset(&sset, SIGSEGV);
    sigaddset(&sset, SIGBUS);
    sigaddset(&sset, SIGABRT);
    sigaddset(&sset, SIGILL);
    sigaddset(&sset, SIGCHLD);
    sigaddset(&sset, SIGFPE);
    sigprocmask(SIG_UNBLOCK, &sset, &sset);

    signal(SIGUSR1, signal_kill);
}


int main(int argc, char** argv)
{
CProcStatePost::Init(CHECK_STATE_FILE);
CProcStatePost::SetState(EN_PROC_STATE_STARTING);
    // 初始化信号量
    g_flag = 0;
    InitSignal();

    int ret_flag = 0;

    // 初始化
    ret_flag = CGlobalServ::Init();
    assert(ret_flag == 0);

    // 启动线程
    ret_flag = CGlobalServ::Start();
    assert(ret_flag == 0);

    // 处理信号相关
CProcStatePost::SetState(EN_PROC_STATE_RUNNING);
    while (1)
    {
        // 收到信号则处理,否则一直等待
        if (g_flag == -1)
        {
            TSE_LOG_INFO(CGlobalServ::m_serv_log, ("Receive stop signal!"));
            TSE_LOG_INFO(CGlobalServ::m_serv_log, ("Stop new req, wait to exit ..."));

            //收到信号之后停止服务, 内部会延时
CProcStatePost::SetState(EN_PROC_STATE_STOPPING);
            CGlobalServ::StopNet();

            TSE_LOG_INFO(CGlobalServ::m_serv_log, ("Stop success!"));
            break;
        }

        //动态更新日志level
        if (access(SERV_LOG_LEVEL_CHANGE_FLAG, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_serv_log, ("reload serv_log conf!"));
            INIT_LOG_MODULE("../conf/serv_log.conf");
            UNUSED(config_ret);
            remove(SERV_LOG_LEVEL_CHANGE_FLAG);
        }

        // 更新module配置
        if (access(DEFAULT_CONF_UPDATE_FLAG, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_serv_log, ("Update module conf!"));
            CConfBase::Update();
            CGlobalServ::m_serv_conf->Update();
            remove(DEFAULT_CONF_UPDATE_FLAG);
        }

        // 更新db配置
        if (access(DB_CONF_UPDATE_FLAG, F_OK) == 0)
        {
            TSE_LOG_INFO(CGlobalServ::m_serv_log, ("Update db conf!"));
            CConfBase::Update();
            CAbsConf::GetInstance()->Update(CGlobalServ::m_serv_log);
            remove(DB_CONF_UPDATE_FLAG);
        }

        // 更新服务路由规则
        CGlobalServ::UpdateRoute();
        sleep(1);
    }

    return 0;
}

