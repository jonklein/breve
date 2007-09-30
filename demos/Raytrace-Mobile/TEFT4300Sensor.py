
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class TEFT4300Sensor( breve.IRSensor ):
	def __init__( self ):
		breve.IRSensor.__init__( self )

	def registerSensorTEFT4300( self ):
		distance = breve.objectList()
		distance_factor = breve.objectList()
		distance_length = 0
		azimut = breve.objectList()
		azimut_factor = breve.objectList()
		azimut_length = 0
		incidence = breve.objectList()
		incidence_factor = breve.objectList()
		incidence_length = 0
		rows = 0
		columns = 0
		maxrange = 0
		maxangle = 0

		rows = 7
		columns = rows
		maxrange = 40
		maxangle = ( ( 60 / 3.141593 ) * 180 )
		distance = [ 0, 2.500000, 5, 7.500000, 10, 12.500000, 15, 17.500000, 20, 22.500000, 25, 30, 35, 40, 50 ]
		distance_length = 15
		distance_factor = [ 1.000000, 0.617020, 0.182980, 0.089360, 0.051060, 0.034040, 0.021280, 0.017020, 0.012770, 0.008510, 0.008510, 0.004260, 0.004260, 0.004260, 0.004260, 0, 0 ]
		azimut = [ ( -65 * 0.017453 ), ( -60 * 0.017453 ), ( -30 * 0.017453 ), ( -20 * 0.017453 ), ( -10 * 0.017453 ), ( 0 * 0.017453 ), ( 10 * 0.017453 ), ( 20 * 0.017453 ), ( 30 * 0.017453 ), ( 60 * 0.017453 ), ( 65 * 0.017453 ) ]
		azimut_factor = [ 0.000000, 0.000000, 0.636400, 0.818200, 1.000000, 1.000000, 1.000000, 0.818200, 0.636400, 0.000000, 0.000000 ]
		azimut_length = 11
		incidence = [ ( -100 * 0.017453 ), ( -90 * 0.017453 ), ( -80 * 0.017453 ), ( -70 * 0.017453 ), ( -60 * 0.017453 ), ( -50 * 0.017453 ), ( -40 * 0.017453 ), ( -30 * 0.017453 ), ( -20 * 0.017453 ), ( -10 * 0.017453 ), ( 0 * 0.017453 ), ( 10 * 0.017453 ), ( 20 * 0.017453 ), ( 30 * 0.017453 ), ( 40 * 0.017453 ), ( 50 * 0.017453 ), ( 60 * 0.017453 ), ( 70 * 0.017453 ), ( 80 * 0.017453 ), ( 90 * 0.017453 ), ( 100 * 0.017453 ) ]
		incidence_factor = [ 0.000000, 0.000000, 0.142900, 0.142900, 0.285700, 0.428600, 0.571400, 0.571400, 0.714300, 0.857100, 1.000000, 0.857100, 0.714300, 0.571400, 0.571400, 0.428600, 0.285700, 0.142900, 0.142900, 0.000000, 0.000000 ]
		incidence_factor = [ 0.000000, 0.000000, 0.214300, 0.285700, 0.357100, 0.500000, 0.642900, 0.714300, 0.785700, 0.928600, 1.000000, 0.928600, 0.785700, 0.714300, 0.642900, 0.500000, 0.357100, 0.285700, 0.214300, 0.000000, 0.000000 ]
		incidence_length = 21
		print '''IRSensor.tz: Creating sensor TEFT4300...'''
		breve.breveInternalFunctionFinder.createUserSensor( self, 'TEFT4300', rows, columns, maxrange, maxangle, distance_length, distance, distance_factor, azimut_length, azimut, azimut_factor, incidence_length, incidence, incidence_factor )
		print '''...sensor created!'''


breve.TEFT4300Sensor = TEFT4300Sensor
# Add our newly created classes to the breve namespace

breve.TEFT4300Sensors = TEFT4300Sensor



