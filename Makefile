all: Frontend Backend run

Frontend: Frontend.cpp
	clang++ -std=c++11 Frontend.cpp -o Frontend

Backend: Backend.cpp
	clang++ -std=c++11 Backend.cpp -o Backend

run:
	clear
	./Frontend Source.txt asm.txt
	./Backend asm.txt MachineCode.txt

clean:
	rm -f Frontend Backend
