
q1: 	
	g++ --std=c++11 -pthread Questão\ 1/q1.cpp -o Questão\ 1/q1
	cd Questão\ 1/ && ./q1 && rm q1
		
q2: 	
	g++ --std=c++11 -pthread Questão\ 2/q2.cpp -o q2
	./q2
	rm q2
	

q3: 
	g++ --std=c++11 -pthread Questão\ 3/q3.cpp -o q3
	./q3
	rm q3

q4: 
	g++ --std=c++11 -pthread Questão\ 4/q4.cpp -o q4
	./q4
	rm q4

q5: 
	g++ --std=c++11 -pthread Questão\ 5/q5.cpp -o Questão\ 5/q5
	cd Questão\ 5/ && ./q5 && rm q5

q6: 
	echo Foi mal teacher
	echo :D

q7: 
	gcc -pthread Questão\ 7/q7.c -o q7
	./q7
	rm q7
push:
	git status
	git add -A
	git commit -m "revisando"
	git push