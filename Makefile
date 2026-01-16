clean:
	rm bin/*

all:
	g++ src/database.cpp src/UserService.cpp test/UserServiceTest.cpp -I include -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient -lcrypto -o bin/UserServiceTest
	g++ src/LogService.cpp test/LogServiceTest.cpp -I include -o bin/LogServiceTest
	g++ test/SignServiceTest.cpp -I include -lcrypto -o bin/SignServiceTest
	g++ src/database.cpp src/OrderService.cpp test/OrderServiceTest.cpp -I include -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient -ldl -o bin/OrderServiceTest

