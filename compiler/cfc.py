#
#	Color Forth compiler (no colour, obviously)
#
from vmcompilerinfo import VMCompilerInfo
import re

class CompilerException(Exception):
	pass
#
#	Represents the memory block.
#
class Memory:
	def __init__(self):
		self.memory = [ None ] * 65536
		self.lowest = 0xFFFF
		self.highest = 0x0000
		self.emit = False

	def write(self,address,data):
		if self.emit:
			print("{0:04x}:{1:02x} {2:c}".format(address,data,(data^32)+32))
		self.lowest = min(self.lowest,address)
		self.highest = max(self.highest,address)
		assert self.memory[address] is None,"Double write to {0:04x}".format(address)
		assert data >= 0 and data < 256,"Bad data value {0:02x}".format(data)
		self.memory[address] = data

	def writeFile(self,name):
		bin = [x if x is not None else 0 for x in self.memory[:self.highest]]
		bin = bytes(bin)
		open(name,"wb").write(bin)

#
#	Represents the dictionary.
#
class Dictionary:
	def __init__(self,coreWords):
		self.dictionaryWords = []
		self.names = {}
		for n in range(0,len(coreWords)):
			self.dictionaryWords.append({ "name":coreWords[n].lower(),"type":"C", "id": n})
			self.names[coreWords[n].lower()] = n

	def contains(self,word):
		return word.lower().strip() in self.names

	def get(self,word):
		return self.dictionaryWords[self.names[word.lower().strip()]]

	def define(self,name,address,isMacro):
		name = name.lower().strip()
		assert name not in self.names and name != "","Bad name '"+name+"'"
		assert address % 2 == 0,"definition address must be even '"+name+"'"
		self.names[name] = len(self.dictionaryWords)
		self.dictionaryWords.append({"name":name,"addroffset":address,"type":"M" if isMacro else "D"})

	def write(self,memory,address):
		currentAddress = address
		for n in range(0,len(self.dictionaryWords)):
			dictWord = self.dictionaryWords[n]
			name = dictWord["name"]
			if name[:2] != "__":
				#print("------")
				# one byte offset to next.
				memory.write(currentAddress,4 + len(name))
				# address or id of word. Note address is in absolute format.
				wAddr = dictWord["addroffset"] if dictWord["type"] != "C" else dictWord["id"]
				memory.write(currentAddress+1,wAddr & 0xFF)
				memory.write(currentAddress+2,wAddr >> 8)
				# length (0..4) macro (6) code word (7)
				info = len(dictWord["name"])
				if dictWord["type"] == "C":
					info = info | 0x80
				if dictWord["type"] == "M":
					info = info | 0x40
				memory.write(currentAddress+3,info)
				# name of word in 6 bit ASCII
				for m in range(0,len(dictWord["name"])):
					memory.write(currentAddress+4+m,ord(dictWord["name"].upper()[m]) & 0x3F)
				#print(name)
				currentAddress = currentAddress + 4 + len(dictWord["name"])

		# 0 end of list
		memory.write(currentAddress,0)
		print("Dictionary ends at {0:04x}".format(currentAddress))
#
#	Represents one compiling instance.
#
class Compiler:
	def __init__(self,compilerInfo):
		self.compilerInfo = compilerInfo
		self.dictionary = Dictionary(compilerInfo.getWordList())
		self.memory = Memory()
		self.pointer = compilerInfo.getCodeBase() + 6
		self.compileEnabled = True
		self.controlWords = { "if":0,"then":0,"repeat":0,"until":0,"for":0,"next":0 }

	def compileFile(self,source):
		print("Compiling '{0}'".format(source))
		src = "\n".join(open(source).readlines())
		self.compile(src)

	def compile(self,code):
		# tabs and spaces
		code = code.replace("\t"," ").replace("\n"," ").lower()
		# remove comments.
		while code.find("{") >= 0:
			p1 = code.find("{")
			p2 = code[p1:].find("}")
			code = code[:p1]+code[p1+p2+1:]
		code = [x.strip() for x in code.split(" ") if x.strip() != ""]
		for word in code:
			self.compileWord(word)

	def compileWord(self,word):
		# handle .endif and .if
		if word[:4] == ".if:":
			self.compileEnabled = not self.dictionary.contains(word[4:])
			return
		if word == ".endif":
			self.compileEnabled = True
			return
		if not self.compileEnabled:
			return	
		# structures.
		if word in self.controlWords:
			self.compileControl(word)
			return
		# handle words in dictionary.
		if self.dictionary.contains(word):
			self.compileCall(word)
			return
		# handle numbers in base 10.
		if re.match("^(\-?[0-9]+)$",word) is not None:
			self.compileConstant(int(word) & 0xFFFF)
			return
		# handle numbers in base 16.
		if re.match("^\$([0-9a-f]+)$",word) is not None:
			self.compileConstant(int(word[1:],16))
			return
		# defines and macros
		if word[0] == ':':
			self.compileDefinition(word[1:])
			return
		# variables and arrays
		if word == "variable":
			self.compileVariable(2)
			return
		if word[:6] == "array.":
			self.compileVariable(int(word[6:]))
			return

		raise CompilerException("Unknown word '"+word+"'")

	def compileConstant(self,value):
		self.compileWord("[literal]")
		self.writeCode(value)

	def compileCall(self,word):
		wInfo = self.dictionary.get(word)
		if wInfo["type"] == "M":
			print("Warning ! Used macro in python compiler '"+wInfo["name"]+"'")
		if wInfo["type"] == "C":
			self.writeCodeByte(wInfo["id"])
		else:
			offset = wInfo["addroffset"] >> 1
			self.writeCodeByte((offset >> 8) | 0x80)
			self.writeCodeByte(offset & 0xFF)

	def compileDefinition(self,word):
		if self.pointer % 2 != 0:
			self.compileWord("[nop]")
		isMacro = False
		#print("{0} {1:04x}".format(word,self.pointer))
		if word[0] == ':':
			isMacro = True
			word = word[1:]
		self.dictionary.define(word,self.pointer-self.compilerInfo.getCodeBase(),isMacro)

	def compileControl(self,word):
		if word == "if":
			self.compileCall("[bzero]")
			self.ifLink = self.pointer
			#print("{0:x} {1:x}".format(self.ifLink,self.pointer))
			self.pointer += 2
			return
		if word == "then":
			offset = self.pointer - (self.ifLink + 2)
			self.memory.write(self.ifLink,offset & 0xFF)
			self.memory.write(self.ifLink+1,offset >> 8)
			self.ifLink = -1
			return

		if word == "repeat":
			self.doLink = self.pointer
			return 
		if word == "until":
			self.compileCall("[bzero]")
			source = self.pointer + 2
			self.writeCode((self.doLink-source) & 0xFFFF)
			self.doLink = -1
			return

		if word == "for":
			self.compileCall(">r")
			self.forLink = self.pointer
			return
		if word == "next":
			self.compileCall("[next]")
			self.compileCall("[bzero]")
			source = self.pointer + 2
			self.writeCode((self.forLink-source) & 0xFFFF)
			self.forLink = -1
			return

		assert False,"not implemented "+word

	def compileVariable(self,size = 2):
		self.compileWord("[variable]")
		for i in range(0,int(size/2)):
			self.writeCode(0)

	def writeCode(self,value):
		#self.memory.emit = True
		self.memory.write(self.pointer,value & 0xFF)
		self.memory.write(self.pointer+1,value >> 8)
		self.pointer += 2

	def writeCodeByte(self,value):
		#self.memory.emit = True
		self.memory.write(self.pointer,value & 0xFF)
		self.pointer += 1

	def complete(self):
		print("Code ends at {0:04x}".format(self.pointer))
		offset = self.pointer - self.compilerInfo.getCodeBase()
		self.memory.write(self.compilerInfo.getCodeBase(),offset & 255)
		self.memory.write(self.compilerInfo.getCodeBase()+1,offset >> 8)

		self.memory.emit = False
		self.pointer = self.compilerInfo.getCodeBase()+2
		self.compileCall("_main")
		self.compileCall("[halt]")
		self.memory.emit = False
		self.dictionary.write(self.memory,self.compilerInfo.getDictionaryBase())

cmp = Compiler(VMCompilerInfo())
cmp.compileFile("group1.cforth")
cmp.compileFile("group2.cforth")
cmp.compileFile("group3.cforth")
cmp.compileFile("group4.cforth")
cmp.compileFile("coreio.cforth")
cmp.compileFile("editor.cforth")

cmp.compile("""

""")

cmp.complete()
cmp.memory.writeFile("cfboot.bin")