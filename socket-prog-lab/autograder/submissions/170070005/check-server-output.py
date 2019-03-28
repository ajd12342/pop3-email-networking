import sys

def RepresentsInt(s):
    try: 
        v = int(s)
        return v > 0
    except ValueError:
        return False

filename1 = sys.argv[1]
filename2 = sys.argv[2]
port = sys.argv[3]
port = port
content1 = []
content2 = []

with open(filename1, "r") as f:
	content1 = f.read().splitlines()

with open(filename2, "r") as f:
	content2 = f.read().splitlines()

for i in range(len(content1)):
	content1[i] = content1[i].replace("\t","").replace(" ","");

for i in range(len(content2)):
	content2[i] = content2[i].replace("\t","").replace(" ","");

ans = "correct"
if(len(content1) != len(content2)):
	ans = "incorrect"
	print(1)
else:
	for i in range(len(content1)):
		csp1 = content1[i].split(":")
		csp2 = content2[i].split(":")
		if len(csp2) != len(csp1):
			print(3)
			ans = "incorrect"
			break
		if i < 2:
			if not (csp1[0] == csp2[0] and port == csp2[1]):
				print(4)
				print(csp1[0] == csp2[0])
				print(csp2[1])
				print(port)
				ans = "incorrect"
				break
		else:
			if len(csp1) < 3:
				if content1[i] != content2[i]:
					ans = "incorrect"
					print(2)
					break
			else:
				if not (csp1[0] == csp2[0] and csp1[1] == csp2[1] and RepresentsInt(csp2[2])):
					print(6)
					ans = "incorrect"
					break

print(ans)


