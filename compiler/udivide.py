#
#	divide 16 bit by 16 bit unsigned.
#
numerator = 31231
denominator = 1231

print(int(numerator/denominator),numerator % denominator)

quotient = 0
remainder= 0

for n in range(0,16):
	# shift numerator left, top bit shifted into remainder which is also shifted left
	remainder = (remainder << 1) & 0xFFFF
	if (numerator & 0x8000) != 0:
		remainder = remainder + 1
	numerator = (numerator << 1) & 0xFFFF

	# shift quotient left
	quotient = quotient << 1
	# if remainder-denominator > 0 subtract and set bit 1 of quotient
	if remainder >= denominator:
		remainder= remainder- denominator
		quotient = quotient | 1

print(quotient,remainder)