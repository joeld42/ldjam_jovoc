import json
import os, sys

# FIXME: read this from the script.txt
PHRASES = {
  'ARTHUR' : [
         "Oh well, if we’re stuck on this yacht at least the bar is well stocked.",
         "Care for a pint? [Maybe] a [glass] of [wine]? [Maybe] a burger or some [fish] and chips?",
         "I thought I heard [water] running in the [wine] cellar. Do you ever worry about the [rain] and flooding?",
         "I thought I heard the calling of a [seabird], but it was just the [generator].",
         "[Water] flows under everything, here.",
  ],
  'JANET' : [
         "I can't believe he cut off communication! This is practically kidnapping, I don't care what the contract says.",
         "It looks like it might [rain] soon. Good for the [sunflower], the [town], and the [world], but I need to fix that [generator].",
         "Saw a [seabird] circling. I need to get back inside before the [rain]. I deserve a sip of [wine].",
         "Lightning is dangerous for the tall [sunflower], but we need the electricity. Am I right?",
         "A [sunflower] is not a [generator]. If you call to a [fish] it will never [answer].",
  ],
  'TULIO' : [
         "We outnumber him. It's not a mutiny if he's not actually a captian, just a crazy billionaire.",
         "I'm just passing through, here. You can keep this little [town]. Not much going on, it's not even on the [map].",
         "[Fish] for dinner, everyday. I'd rather eat a [sunflower], I'm so tired of this [world].",
         "Janet says [violence] is not the [answer], but sometimes the [answer] is [violence], or [maybe] [wine].",
         "I'm going to sink this [town] if I can't get off of here.",
  ],
  'ESME' : [
         "Don't be so afraid of the captian. He just needs a few more days to calm down.",
         "Everyone in [town] is worried about the [rain], but I'm like a [fish], I love the [water].",
         "I bought a [sunflower] in the shop today to brighten up the [glass] in my window.",
         "A [town] is made of people, not buildings or rooms. We're all in this [boat] together, even the [captain].",
         "I'm more afraid of being alone in the [world] than I am of drowning or [violence]. There is no [answer], we are lost at sea, all of us.",
  ],
  'JAMAL' : [
         "What use is a navigator if he won't turn on the engines?",
         "Just ask if you need a [map]. Oh! I seem to have misplaced it.",
         "The [captain] says one can't be lost if one burns their [map].",
         "I'm worried that some in the [town] might resort to [violence] if we can't talk sense into the [captain].",
         "A [glass] of [wine] made a glyph like a [sunflower] on my [map]. Is that the [answer]?",
  ],
  'SANDY' : [
         "There are worse places to be stranded than a fancy yacht surrounded by enchanting gulls.",
         "(Humming) I called up the [captain], said please bring me my [wine] ...",
         "So pretty, like [fish] beneath a [glass] bottom [boat].",
         "Tulio says he feels trapped here, but where would he go? We are all trapped on this [world].",
         "Sometimes the [answer] is to enjoy the [rain], like a [seabird].",
  ],
}


class TileLevel( object ):

	def __init__(self):

		self.identifier = "Unnamed"
		self.worldPos = ( 0, 0 )
		self.layers = []
		self.collision = []
		self.tiles = []
		self.journalText = None
		self.journalPos = ( 0,0, 0, 0)
		self.sleeps = []
		self.actor = None
		self.actorPos = (0,0)

	@staticmethod
	def from_dict( data ):

		level = TileLevel()

		level.identifier = data.get( 'identifier', "Unnamed" )
		wx = int(data.get('worldX'))
		wy = -int(data.get('worldY'))
		level.worldPos = ( int(wx / 25), int(wy/25) )

		collisionLayerData = None
		floorTilesLayerData = None
		entityLayerData = None

		for layerData in data.get( 'layerInstances'):
			if layerData.get( "__identifier") == "Collision":
				collisionLayerData = layerData
			elif layerData.get( "__identifier") == "FloorTiles":
				floorTilesLayerData = layerData
			elif layerData.get("__identifier") == "Entities":
				entityLayerData = layerData

		if collisionLayerData is None:
			print ("Missing Collision layer")
			sys.exit(1)

		# Unpack collision data
		w = int(collisionLayerData.get("__cWid"))
		h = int(collisionLayerData.get("__cHei"))
		if w != 16 or h != 11:
			print( f"WARNING: unexpected size ${w} ${h}")

		layerSize = w * h
		level.collision = collisionLayerData.get("intGridCsv")
		if (len(level.collision)!=layerSize):
			print("Unexpected size intGridCsv")

		TSZ = 25
		for tile in floorTilesLayerData.get( 'gridTiles' ):
			srcTile = tile.get("src")
			px = tile.get("px")

			print (px, srcTile)

			# Logical tileset is 4100x4100 so the last row/col get cut off
			# a few pixels here so don't put important items in the last edge
			st0_raw = (srcTile[0] , srcTile[1])
			st1_raw = (srcTile[0] + TSZ, srcTile[1] + TSZ)
			flags = tile.get("f")
			#print ("Tile ", px[0]/TSZ, px[1]/TSZ, " ST ", st0 , st1)
			if (flags==0):
				# No flip
				st0 = st0_raw
				st1 = st1_raw
			elif (flags==1):
				# Flip horizontal
				st0 = ( st1_raw[0], st0_raw[1] )
				st1 = ( st0_raw[0], st1_raw[1])
			elif (flags==2):
				# Flip vertical
				st0 = (st0_raw[0], st1_raw[1])
				st1 = (st1_raw[0], st0_raw[1])
			elif (flags==3):
				# flip both
				st0 = st1_raw
				st1 = st0_raw

			tileData = ( int(px[0]/TSZ), -int(px[1]/TSZ), (st0[0] / 1025.0, st0[1] / 1025.0), (st1[0] / 1025.0, st1[1] / 1025.0) )
			#print (tileData)
			level.tiles.append( tileData )

		# Unpack the level entities
		for ent in entityLayerData.get( 'entityInstances', []):
			etype = ent.get( '__identifier', "unk" )
			print ( "entity", etype )
			if (etype == 'JournalText'):
				px, py = ent['__grid']
				w = int(ent['width'] / 25)
				h = int(ent['height'] / 25)
				storyText = "text missing"
				for ff in ent['fieldInstances']:
					fid = ff['__identifier']
					if fid == 'StoryText':
						storyText = ff['__value']

				print( "text ", px, py, w, h, storyText )
				if not level.journalText is None:
					print( "WARN: level already has journal text")

				level.journalText = storyText
				level.journalPos = ( px, py,  w,  h )

			elif (etype == 'Sleep'):
				px, py = ent['__grid']
				w = int(ent['width'] / 25)
				h = int(ent['height'] / 25)
				asleepHere = False
				for ff in ent['fieldInstances']:
					fid = ff['__identifier']
					if fid == 'asleepHere':
						asleepHere = ff['__value']

				level.sleeps.append( (px, py, w, h, asleepHere) )

			elif (etype == 'Actor'):
				px, py = ent['__grid']
				name = "UNKNOWN"
				for ff in ent['fieldInstances']:
					fid = ff['__identifier']
					if fid=='ActorName':
						name = ff['__value']

				if name != "Player":
					level.actor = name
					level.actorPos = ( px, py )
					print ("Found actor", name )

		return level

class TileWorld( object ):

	def __init__( self ):
		self.levels = []

	@staticmethod
	def from_dict( data ):

		world = TileWorld()

		#print( data.keys() )
		for leveldata in data.get('levels', []):
			level = TileLevel.from_dict( leveldata )

			world.levels.append( level )

		return world

def dumpLevelData( world ):

	fp = open("level-data.cpp", "wt")
	fp.write("/* Auto-generated, do not edit */\n\n\n")

	for level in world.levels:
		fp.write( f"RoomInfo room_{level.identifier} = {{\n")
		fp.write( f'   .name = "{level.identifier}",\n')
		fp.write( f'    .worldX = {level.worldPos[0]},\n')
		fp.write(f'    .worldY = {level.worldPos[1]},\n')
		fp.write( f'   .collision = {{\n')
		for row in range(11):
			rowCollision = level.collision[ 16*row : 16*(row+1) ]
			fp.write( '    '+ ', '.join( map( lambda x: str(x), rowCollision )) + ',\n' )
		fp.write( f'   }},\n')

		count = 0
		fp.write(f'   .numTiles = {len(level.tiles)},\n')
		fp.write(f'   .tiles = {{\n')
		for tile in level.tiles:
			fp.write(f'     {{ .tx = {tile[0]}, .ty = {tile[1]}, .st0 = {{ { tile[2][0] }, { tile[2][1] } }}, .st1 = {{ { tile[3][0] }, { tile[3][1] } }} }}, \n')
		fp.write(f'   }},\n')

		if level.journalText:
			fp.write( f'    .journal = {{\n')
			fp.write( f'        .rect = {{ .tx = { level.journalPos[0] }, .ty = { level.journalPos[1] }, .w = { level.journalPos[2] }, .h = { level.journalPos[3] } }},\n')
			fp.write( f'        .text = ');
			for line in level.journalText.split('\n'):
				line = line.strip()
				fp.write(f'\n        "{line}\\n"');
			fp.write(f',\n     }},\n')

		if len( level.sleeps ):
			fp.write(f'    .sleeps = {{\n')
			for s in level.sleeps:
				sleepText = ""
				if (s[4]):
					sleepText = ", .asleepHere = true"
				fp.write( f'          {{ .rect = {{ .tx = {s[0]}, .ty = {s[1]}, .w = {s[2]}, .h = {s[3]}  }}{sleepText} }},\n')
			fp.write(f'     }},\n')


		if level.actor:
			phrases = PHRASES.get(level.actor.upper(), [])
			if (len(phrases) != 5):
				print ("ERROR: missing or wrong number of phrases for ", level.actor );

			fp.write(f'    .actor = {{\n')
			fp.write(f'        .name = "{level.actor}",\n')
			fp.write(f'        .tx = {level.actorPos[0]}, .ty = {level.actorPos[1]},\n')
			fp.write(f'        .phrase = {{\n');
			for phrase in phrases:
				# HACK: fix intentional misspealling
				phrase = phrase.replace( "aptian", "aptain")
				fp.write(f'            "{phrase}",\n');
			fp.write(f'        }},\n');
			fp.write(f'    }},\n')



		fp.write(f"}};\n\n\n")

	# Now the world info
	fp.write(f"WorldMap world = {{\n")
	fp.write(f"    .rooms = {{\n" )
	for level in world.levels:
		fp.write(f"             &room_{level.identifier},\n")
	fp.write(f"             NULL }},\n")
	fp.write(f"}};\n\n\n")

	fp.close()

if __name__=='__main__':
	
	# test
	with open('./srcart/Deeper.ldtk') as fp:
		data = json.load(fp)

		world = TileWorld.from_dict( data )

	dumpLevelData( world )
	for lvl in world.levels:
		print( f"Level {lvl.identifier} pos {lvl.worldPos}" )
