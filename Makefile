
.PHONY: build check help version client

help:
	@echo "支持下面命令:"
	@echo "make build 编译项目"
	@echo "make check 代码静态检查"

build:
	g++ -g -pthread -o test main.cpp ./src/*.cpp -I ./include

check:
	cppcheck ./

client:
	g++ -g -pthread -o client client.cpp ./src/*.cpp -I ./include

version:
	@git log --pretty=format:"%h%x09%an%x09%ad%x09%s" --date=relative -20

