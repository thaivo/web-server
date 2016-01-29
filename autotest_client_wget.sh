#!/bin/sh

a=0
#eval cd /home/local/INI/thai.vo/workspace/DemoC
while [ $a -lt 100 ]
do
 a=`expr $a + 1`
  eval wget "--post-data=""'op=insert&id='$a'&value=thai'" "http://localhost:8000 &" 
	
done

