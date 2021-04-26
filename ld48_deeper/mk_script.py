import os, sys
import string

SKIPWORDS = []
WORDS = {}

SPEAKERS = {}

def sanitize( w ):
	w2 = ""
	for ch in w:
		if ch.isalpha():
			w2 = w2 + ch.upper()

	return w2

def process(w):

	w2 = sanitize(w)

	if ((len(w2) > 3) or (w2 in ['MAP'])  )and (not w2 in SKIPWORDS):
		c = WORDS.get( w2, 0 )
		WORDS[w2] = c + 1;

for line in open("script.txt", "rt").readlines():
	if (line.startswith("__end")):
		break

	words = line.split()
	#print(line)
	
	if len(words) > 0 and words[0] == 'SKIPWORDS':
		SKIPWORDS = words[2:]
		continue

	if len(words):
		speaker = words[0]
		if not speaker in SPEAKERS:
			SPEAKERS[speaker] = []

		SPEAKERS[ speaker ].append( line[len(speaker):].strip() )

	for w in words[1:]:
		process(w)

wlist = list(WORDS)
wlist.sort()
DREAMWORDS = []
count = 0
print (wlist)
for w in wlist:
	c = WORDS[w]
	if c >= 3:
		count += 1
		print(count, WORDS[w], w)
		DREAMWORDS.append( w )

print("PHRASES = {")
for actor in SPEAKERS.keys():	
	print( f"  '{actor}' : [")
	for phrase in SPEAKERS[actor]:

		# highlight the dream words
		phrase2 = []
		for w in phrase.split(' '):
			w2 = sanitize(w)
			if w2 in DREAMWORDS:
				if not w[-1].isalpha():
					phrase2.append(f"[{w[:-1]}]{w[-1]}")
				else:
					phrase2.append( f"[{w}]")
			else:
				phrase2.append( w )
		phrase2text = ' '.join(phrase2)

		print( f'         "{phrase2text}",' )
	print( f"  ],")
print("}")

