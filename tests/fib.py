def fib(i):
	if i < 2.0: return i
	else: return fib(i - 2.0) + fib(i - 1.0)

print(fib(30.0))