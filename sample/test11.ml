globalvar <- 0

function addglobal n
	globalvar <- globalvar + n
	return globalvar

print addglobal(5)
print globalvar