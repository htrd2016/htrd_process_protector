# htrd_process_protector

简介：该项目的目的是为了检测salt-minion超时不执行master，实现机制：master上会定时发命令在minion上执行写文件命令，监控进程发现该文件超时未修改文件就去重启服务。

（1）文件夹htrd_linux_process_protector 是linux上运行的守护程序工程。
     不同系统需要重编译一次，在测试过程中生成的文件需要重命名，目前命名规则是redhat5.8为htrdMinionProtector-el5，centos7为htrdMinionProtector-el7，
     启动方式：/root/minion_protector/htrdMinionProtector-el5 /tmp/htrd_minion_alived "service salt-minion stop" "service salt-minion start" 300，
     具体含义为：进程名 需要定时检测的文件名 停掉服务的命令 启动服务的命令 文件未被修改的超时时间。
     
（2）文件夹htrd_windows_process_protector 是windows上运行的守护程序工程，
      设置为静态编译（vs的“配置属性”-》“C/C++”-》“代码生成”的“运行库”设置为“多线程(/MT)”）
      启动方式：C:\\minion_protector\\htrdMinionProtector.exe %temp%\\htrd_minion_alived "net stop salt-minion" "net start salt-minion" 300
      具体含义:进程名 需要定时检测的文件名 停掉服务的命令 启动服务的命令 文件未被修改的超时时间。
      
（3）文件夹salt_master_shell里面是需要在salt-master上执行的脚本
    1、copy_exe_to_salt_base.sh：是将生成的可执行程序拷贝到salt的文件服务器目录以便下发文件使用
    （前提是将linux和windows生成的可执行程序放到minion_protector目录下，其中系统redhat5.8的可执行文件名为htrdMinionProtector-el5，
    Centos7的可执行程序名为htrdMinionProtector-el7，windows下的可执行程序名为htrdMinionProtector.exe）。
    
    2、send_protector_to_minions.sh：是通过salt-master将文件下发到minion上（如果是linux系统，需要通过chmod +x 赋予可执行权限），
      目前只在redhat5.8和windows2003上做了测试。
      
    3、write_alived.sh：是需要在salt-master所在的系统上设置定时任务调用的脚本（测试的时候定时任务为1分钟一次），
       该脚本用于定时给minion发写指定文件的命令，以便守护进程判断minion是否还活着，如果守护进程发现该文件超时未被修改，会通过命令重启minion服务。
       
    4、run_monion_protecter.sh：是通过salt-master将守护进程启动起来。
--------------------------------------------------------------------------------------------------------------------------
目前测试情况：测试环境42.62.125.189上的虚拟机上，salt-master的ip地址是10.10.131.201，redhat5.8测试系统的ip是10.10.131.203。
             windows2003的ip地址10.10.250.230。

在redhat5.8上的已经做的测试步骤：
   1.master启动定时器执行write_alived，在master上手动运行run_monion_protecter.sh执行守护进程，
     通过top命令观察守护进程的资源消耗比较稳定
   2.在步骤1的基础上杀掉salt-minion进程，过一会salt-minion进程被守护进程启动起来，
     通过top命令观察守护进程的资源消耗比较稳定
   3.在步骤1的基础上杀掉salt-minion进程，将虚拟机网卡设置为nat模式，使redhat5.8和master无法通讯，
     一段时间salt-minion会重启（通过ps -ef | grep "salt-minion"查看进程id是否与之前相同），通过top命令观察守护进程的资源消耗比较稳定。
   
在windows2003上的测试步骤：
   1.master启动定时器执行write_alived，在master上手动运行run_monion_protecter.sh执行守护进程，
     通过资源管理器观察守护进程的资源消耗比较稳定。
   2.在步骤1的基础上结束salt-minion服务，过一会salt-minion进程被守护进程启动起来，
     通过资源管理器观察守护进程的资源消耗比较稳定。
