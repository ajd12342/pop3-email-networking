import sys

def RepresentsInt(s):
    try: 
        v = int(s)
        return v > 0
    except ValueError:
        return False

filename1 = sys.argv[1]
filename2 = sys.argv[2]
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
else:
	for i in range(len(content1)):
		if i != 0:
			if content1[i] != content2[i]:
				ans = "incorrect"
				break
		else:
			csp1 = content1[i].split(":")
			csp2 = content2[i].split(":")
			if len(csp2) != len(csp1):
				ans = "incorrect"
				break
			else:
				if not (csp1[0] == csp2[0] and csp1[1] == csp2[1] and RepresentsInt(csp2[2])):
					ans = "incorrect"
					break

print(ans)


