salt -G "os:redhat" cp.get_file salt://minion_protector/htrdMinionProtector-el5 /root/minion_protector/htrdMinionProtector-el5 makedirs=True
salt -G "os:redhat" cmd.run_all "chmod +x /root/minion_protector/htrdMinionProtector-el5" 

salt -G "os:CentOS" cp.get_file salt://minion_protector/htrdMinionProtector-el7 /root/minion_protector/htrdMinionProtector-el7 makedirs=True
salt -G "os:CentOS" cmd.run_all "chmod +x /root/minion_protector/htrdMinionProtector-el7"

salt -G "os:windows" cp.get_file salt://minion_protector/htrdMinionProtector.exe C:\\minion_protector\\htrdMinionProtector.exe makedirs=True
