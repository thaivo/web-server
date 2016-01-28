#!/bin/sh
while :
do
   echo "Pres CTRL+C to stop..."
   sleep 1
   o=$(ps cax | grep -c httpd)
   if [ $o -eq 1 ]; then
    	sMess="httpd server successfully restarted"
    	echo $sMess
   else
    	sMess="httpd server FAILED to restart. try to restart httpd server..."
    	echo $sMess
    	/home/local/INI/thai.vo/workspace/httpd/src/httpd
    	if [$? ==  130]
		then
			break       	   #Abandon the loop.
		fi
   fi 
done


