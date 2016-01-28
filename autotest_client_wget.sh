#!/bin/sh

a=0
eval cd /home/local/INI/thai.vo/workspace/DemoC
while [ $a -lt 100 ]
do
 a=`expr $a + 1`
  eval wget "--post-data=""'op=insert&id='$a'&name=thai&dept=rd&jobtitle=eng'" "http://localhost:8000 &" 
	
done

