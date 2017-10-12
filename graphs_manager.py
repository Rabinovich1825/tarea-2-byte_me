#import matplotlib as pyplot

results = open("results.txt", "r")
line = 0
current_program = ""
valores = {}
for data in results:
	if line == 0:			#focus result is -2533
		datos = data.split()
		current_program = datos[0]
		if current_program not in valores:
			valores[current_program] = {}
			valores[current_program]["result"] = []
			valores[current_program]["faults"] = []
			valores[current_program]["writes"] = []
			valores[current_program]["reads"] = []
		valores[current_program]['result'].append(int(datos[3]))
		line += 1
	elif line == 1:			#page faults: 297
		datos = data.split()
		valores[current_program]['faults'].append(int(datos[2]))

		line += 1
	elif line == 2:			#total disk writes: 297
		datos = data.split()
		valores[current_program]['writes'].append(int(datos[3]))
		line += 1
	elif line == 3:			#total disk readings: 199
		datos = data.split()
		valores[current_program]['reads'].append(int(datos[3]))
		line = 0
