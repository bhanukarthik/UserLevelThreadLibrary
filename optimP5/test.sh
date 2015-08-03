 #!/bin/bash
        for i in `seq 1 50`;
        do
		echo $i
                /usr/bin/time -f  "user : %U" ./a.out $i > /dev/null

        done    
