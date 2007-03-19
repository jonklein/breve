
import breve

class IRSensor( breve.Real ):
	''''''

	__slots__ = [ 'communicationThreshold', 'draw', 'm_location', 'm_rotation', 'owner', 'realWorldPointer_owner', 'rel_position', 'rel_rotation', 'sensorType', 'sensordata' ]

	def __init__( self ):
		breve.Real.__init__( self )
		self.communicationThreshold = 0
		self.draw = None
		self.m_location = breve.vector()
		self.m_rotation = breve.matrix()
		self.owner = None
		self.realWorldPointer_owner = None
		self.rel_position = breve.vector()
		self.rel_rotation = breve.matrix()
		self.sensorType = ''
		self.sensordata = 0

	def canSendAck( self, agent ):
		''''''

		qtest = 0

		qtest = ( 233 * breve.breveInternalFunctionFinder.calcQualNoRay( self, self.realWorldPointer_owner, self.getLocation(), self.getRotation(), agent.getLocation(), self.sensorType ) )
		if ( qtest > self.communicationThreshold ):
			return 1


		return 0

	def getData( self ):
		''''''


		self.updatePos()
		if ( self.sensordata == -1 ):
			self.draw.clear()
			self.draw.setColor( breve.vector( 0.100000, 0.100000, 0.100000 ) )
			self.draw.drawLine( self.getLocation(), ( self.getLocation() + ( self.getRotation() * breve.vector( 0, 0, 30 ) ) ) )
			self.sensordata = ( 233 * breve.breveInternalFunctionFinder.irSense( self, self.realWorldPointer_owner, self.getLocation(), self.getRotation(), self.sensorType ) )


		return self.sensordata

	def getLocation( self ):
		''''''


		return self.m_location

	def getOwner( self ):
		''''''


		return self.owner

	def getRotation( self ):
		''''''


		return self.m_rotation

	def getSensorType( self ):
		''''''


		return self.sensorType

	def initWith( self, t, position, rotation, o ):
		''''''


		self.sensorType = t
		self.setRelPosition( position )
		self.setRelRotation( rotation )
		self.owner = o
		self.realWorldPointer_owner = o.getRealWorldPointer()
		self.draw = breve.createInstances( breve.Drawing, 1 )
		self.sensordata = 0
		self.updatePos()
		self.communicationThreshold = 30.000000
		return self

	def iterate( self ):
		''''''


		self.sensordata = -1
		self.updatePos()

	def move( self, location ):
		''''''


		self.m_location = location

	def send( self, message ):
		''''''

		qtest = 0
		agents = breve.objectList()
		i = None
		col = 0

		col = breve.randomExpression( 100 )
		self.updatePos()
		self.draw.clear()
		agents = breve.allInstances( "basicAgent" )
		self.draw.setColor( breve.vector( 1, 0, 0 ) )
		self.draw.drawLine( self.getLocation(), ( self.getLocation() + ( self.getRotation() * breve.vector( 0, 0, 27 ) ) ) )
		for i in agents:
			if ( i != self.owner ):
				qtest = ( 233 * breve.breveInternalFunctionFinder.calculateQualityToObject( self, self.realWorldPointer_owner, self.getLocation(), self.getRotation(), i.getRealWorldPointer(), self.sensorType ) )
				self.draw.setColor( breve.vector( 0, 0, 0 ) )
				if ( qtest > self.communicationThreshold ):
					i.receiveMessage2( message, self.getOwner() )







	def setOwner( self, o ):
		''''''


		self.owner = o

	def setRelPosition( self, v ):
		''''''


		self.rel_position = v

	def setRelRotation( self, m ):
		''''''


		self.rel_rotation = m

	def setRelYRotation( self, f ):
		''''''


		self.setRelRotation( breve.matrix(  breve.breveInternalFunctionFinder.cos( self, f ), 0, ( -breve.breveInternalFunctionFinder.sin( self, f ) ), 0, 1, 0, breve.breveInternalFunctionFinder.sin( self, f ), 0, breve.breveInternalFunctionFinder.cos( self, f ) ) )

	def setRotation( self, m ):
		''''''


		self.m_rotation = m

	def updatePos( self ):
		''''''


		self.move( ( self.owner.getLocation() + ( self.owner.getRotation() * self.rel_position ) ) )
		self.setRotation( ( self.owner.getRotation() * self.rel_rotation ) )


breve.IRSensor = IRSensor
breve.IRSensors = IRSensor



