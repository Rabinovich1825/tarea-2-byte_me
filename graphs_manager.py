#import matplotlib as pyplot

global valores

def funcion(archivo, nombre):
	line = 0
	contador = 0
	algoritmo = "fifo"
	for data in archivo:
		if contador == 99:
			algoritmo = "rand"
		elif contador == 198:
			algoritmo = "custom"
		datos = data.split()
		if line == 0:			#focus result is -2533
			valores[nombre][algoritmo]["results"].append(int(datos[3]))
			line += 1
		elif line == 1:			#page faults: 297
			valores[nombre][algoritmo]['faults'].append(int(datos[2]))
			line += 1
		elif line == 2:			#total disk writes: 297
			valores[nombre][algoritmo]['writes'].append(int(datos[3]))
			line += 1
		elif line == 3:			#total disk readings: 199
			valores[nombre][algoritmo]['reads'].append(int(datos[3]))
			line = 0
			contador += 1
			

focus = open("focus_results.txt", "r")
scan = open("scan_results.txt", "r")
sort = open("sort_results.txt", "r")
valores = {}
valores["focus"] = {}
valores["focus"]["fifo"] = {}
valores["focus"]["fifo"]["results"] = []
valores["focus"]["fifo"]["faults"] = []
valores["focus"]["fifo"]["writes"] = []
valores["focus"]["fifo"]["reads"] = []
valores["focus"]["rand"] = {}
valores["focus"]["rand"]["results"] = []
valores["focus"]["rand"]["faults"] = []
valores["focus"]["rand"]["writes"] = []
valores["focus"]["rand"]["reads"] = []
valores["focus"]["custom"] = {}
valores["focus"]["custom"]["results"] = []
valores["focus"]["custom"]["faults"] = []
valores["focus"]["custom"]["writes"] = []
valores["focus"]["custom"]["reads"] = []

valores["scan"] = {}
valores["scan"]["fifo"] = {}
valores["scan"]["fifo"]["results"] = []
valores["scan"]["fifo"]["faults"] = []
valores["scan"]["fifo"]["writes"] = []
valores["scan"]["fifo"]["reads"] = []
valores["scan"]["rand"] = {}
valores["scan"]["rand"]["results"] = []
valores["scan"]["rand"]["faults"] = []
valores["scan"]["rand"]["writes"] = []
valores["scan"]["rand"]["reads"] = []
valores["scan"]["custom"] = {}
valores["scan"]["custom"]["results"] = []
valores["scan"]["custom"]["faults"] = []
valores["scan"]["custom"]["writes"] = []
valores["scan"]["custom"]["reads"] = []

valores["sort"] = {}
valores["sort"]["fifo"] = {}
valores["sort"]["fifo"]["results"] = []
valores["sort"]["fifo"]["faults"] = []
valores["sort"]["fifo"]["writes"] = []
valores["sort"]["fifo"]["reads"] = []
valores["sort"]["rand"] = {}
valores["sort"]["rand"]["results"] = []
valores["sort"]["rand"]["faults"] = []
valores["sort"]["rand"]["writes"] = []
valores["sort"]["rand"]["reads"] = []
valores["sort"]["custom"] = {}
valores["sort"]["custom"]["results"] = []
valores["sort"]["custom"]["faults"] = []
valores["sort"]["custom"]["writes"] = []
valores["sort"]["custom"]["reads"] = []

funcion(focus, "focus")
funcion(scan, "scan")
funcion(sort, "sort")

for k in valores:
	print k
	for j in valores[k]:
		print "\t"+j
		for i in valores[k][j]:
			print "\t\t"+i, valores[k][j][i]
