
# 首行空行
two <- 2
double <- 2.222 # C关键字冲突
single <- 1 # 行尾注释
double <- 2 # 重复赋值

# 段中空行
function increment1 value
	one <- - 1
	after <- value + one # 参数使用/局部变量使用
	return after
function increment2 value
	before <- value
	value <- two # 全局变量利用
	return before + two # 返回值处运算
function square value
	print value * value # 打印处运算
#
result1 <- increment1(0.123456) # 函数返回值利用
print result1 # 浮点数输出 - 1.123456
result2 <- increment2(0 - arg0) # 命令行参数利用
print result2 # 负数输出测试 - (2 - arg0)
print increment1(2 * 3) # 函数参数处运算 - 7
print increment2(4) + 2 # 打印处调用函数测试 - 8
print increment2(arg2) # 多命令行参数测试 - (arg2 + 2)
square(-0.3) # 函数内部打印测试/负数输入测试 - 0.090000
print -arg0 # 负变量测试 - -3
print -increment1(1) # 负表达式测试 - -2
square((1 + 2) * (4 - 1) / (6 / 2)) # 括号运算测试 - 9

#
# 使用方法: ./runml test12.ml 3 2 1
# 返回结果为: 1.123456-17830.090000-3-29
#
# 段尾无空行测试