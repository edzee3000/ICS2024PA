STUID = 231300027 
STUNAME = 朱士杭

# DO NOT modify the following code!!!

GITFLAGS = -q --author='tracer-ics2024 <tracer@njuics.org>' --no-verify --allow-empty

# prototype: git_commit(msg)
define git_commit
	-@git add $(NEMU_HOME)/.. -A --ignore-errors
	-@while (test -e .git/index.lock); do sleep 0.1; done
	-@(echo "> $(1)" && echo $(STUID) $(STUNAME) && uname -a && uptime) | git commit -F - $(GITFLAGS)
	-@sync
endef

_default:
	@echo "Please run 'make' under subprojects."

submit:
	git gc
	STUID=$(STUID) STUNAME=$(STUNAME) bash -c "$$(curl -s http://175.24.131.173:8080/static/submit.sh)"

count:
	@echo "PA1和PA0相比修改的代码行数为："
	@git diff --numstat pa0..pa1
	@git diff --numstat pa0..pa1 | awk '{total += $$1 + $$2} END {print "Total lines added/removed: " total}'

.PHONY: default submit
