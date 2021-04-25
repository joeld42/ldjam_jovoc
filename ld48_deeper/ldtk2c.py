import json
import os, sys

class TileLevel( object ):

	def __init__(self):

		self.identifier = "Unnamed"
		self.worldPos = ( 0, 0 )
		self.layers = []
		self.collision = []
		self.tiles = []

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
