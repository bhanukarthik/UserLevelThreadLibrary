 #!/bin/bash
        for i in `seq 1 25`;
        do
		echo "Number of Kernel threads" $i ;
		echo -ne  "spinlock"; /usr/bin/time -f  " %U" ./a.out $i >/dev/null
		echo -ne  "optimzed"; /usr/bin/time -f  " %U" optimP5/a.out $i>/dev/null

        done    
