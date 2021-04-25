import os, sys
import string

SKIPWORDS = []
WORDS = {}

def process(w):

	w2 = ""
	for ch in w:
		if ch.isalpha():
			w2 = w2 + ch.upper()

	if (len(w2) > 3) and (not w2 in SKIPWORDS):		
		c = WORDS.get( w2, 0 )
		WORDS[w2] = c + 1;

for line in open("script.txt", "rt").readlines():
	if (line.startswith("__end")):
		break

	words = line.split()
	
	if len(words) > 0 and words[0] == 'SKIPWORDS':
		SKIPWORDS = words[2:]
		continue

	for w in words[1:]:
		process(w)

wlist = list(WORDS)
wlist.sort()
count = 0
for w in wlist:
	c = WORDS[w]
	if c >= 3:
		count += 1
		print(count, WORDS[w], w)