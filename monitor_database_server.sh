    #!/bin/sh
    # Shell script to restart MySQL server if it is killed or not working 
    # due to ANY causes.
    # When script detects mysql is not running (it basically sends ping request 
    # to MySQL) it try to start using /etc/init.d/mysql script; and it sends an 
    # email to user indicating the status.
    # This script must be run from Cron Job so that it can monitor mysql server. 
    # For more info visit following url:
    # http://www.cyberciti.biz/nixcraft/vivek/blogger/2005/08/linux-mysql-server-monitoring.html 
    # --------------------------------------------------------------------------
    # Copyright (C) 2005 nixCraft project <http://cyberciti.biz/fb/>
    # This script is licensed under GNU GPL version 2.0 or above
    # -------------------------------------------------------------------------
    # This script is part of nixCraft shell script collection (NSSC)
    # Visit http://bash.cyberciti.biz/ for more information.
    # -------------------------------------------------------------------------
     
    
    MUSER=$(eval echo $MYSQL_USERNAME)
    # mysql admin/root password
    MPASS=$(eval echo $MYSQL_PASSWORD)
    # mysql server hostname
    MHOST="localhost"
    #Shell script to start MySQL server i.e. path to MySQL daemon start/stop script.
    # Debain uses following script, need to setup this according to your UNIX/Linux/BSD OS. 
    MSTART="sudo /etc/init.d/mysql start"
    # path mysqladmin
    MADMIN="$(which mysqladmin)"
     
    # see if MySQL server is alive or not
    # 2&1 could be better but i would like to keep it simple and easy to
    # understand stuff :)
while :
do
    $MADMIN -h $MHOST -u $MUSER -p${MPASS} ping 2>/dev/null 1>/dev/null
    
	
    if [ $? -ne 0 ]; then
    	echo "Error: MySQL Server is not running/responding ping request"
    	echo "Hostname: $(hostname)" 
    	echo "Date & Time: $(date)"
    	# try to start mysql
    	$MSTART>/dev/null
    	# see if it is started or not
    	o=$(ps cax | grep -c ' mysqld$')
    	if [ $o -eq 1 ]; then
    		sMess="MySQL Server MySQL server successfully restarted"
    	else
    		sMess="MySQL server FAILED to restart"
    	fi
    	echo $sMess
    else # MySQL is running :) and do nothing
    	:
    fi
    # remove file
    rm -f $MAILMESSAGE
done




