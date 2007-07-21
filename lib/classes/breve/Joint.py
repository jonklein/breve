
import breve

class Joint( breve.Abstract ):
	'''Joint a is class used to connect two OBJECT(Mobile) objects together.  The Joint class itself is actually never instantiated--instead, one of the following joint subclasses should be used. <ul> <li>OBJECT(PrismaticJoint) for linear sliding joints between links <li>OBJECT(RevoluteJoint) for rotational joints between links <li>OBJECT(FixedJoint) for static joints between links <li>OBJECT(BallJoint) for ball joints between links <li>OBJECT(UniversalJoint) for ball joints between links </ul> <P> Although the class itself is never instantiated, the class methods  described below are often used with the object's subclasses. <P> The joints supported in breve can be 1, 2 or 3 degrees of freedom  (DOF), meaning that they allow motion in 1, 2 or 3 independent  directions.  '''

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.child = None
		self.clinkPoint = breve.vector()
		self.jointPointer = None
		self.maxVector = breve.vector()
		self.minVector = breve.vector()
		self.normalVector = breve.vector()
		self.parent = None
		self.plinkPoint = breve.vector()
		self.relativeRotation = breve.matrix()
		self.springMax = 0
		self.springMin = 0
		self.springStrength = 0
		self.strengthLimit = 0
		Joint.init( self )

	def snap( self ):
		'''Deprecated -- use break-joint.'''


		self.breakJoint()

	def breakJoint( self ):
		'''Breaks this joint--removes the child (and all of the links connected to it) from the body.  The child link (and its descendents) are therefore removed  from the world and placed in limbo.  They may be reconnected later by  creating another Joint, or it may be associated with a OBJECT(MultiBody) of  its own. <P> The Joint object is <b>not</b> freed&mdash;this has been changed from breve 1.7.  By calling this method, you break the joint without releasing it such that it can be used again.  If you do not with to use the Joint object again, then simply freeing the Joint object will break the joint and delete the object.'''


		self.disableAutomaticJointScaling()
		if self.jointPointer:
			breve.breveInternalFunctionFinder.jointBreak( self, self.jointPointer )

		self.jointPointer = 0

	def dearchive( self ):

		if self.jointPointer:
			self.setStrengthLimit( self.strengthLimit )
			self.setDoubleSpring( self.springStrength, self.springMax, self.springMin )


		return 1

	def destroy( self ):

		self.breakJoint()

	def disableAutomaticJointScaling( self ):
		'''Disables automatic joint scaling, if it had previously been enabled using METHOD(enable-automatic-joint-scaling).'''


		if self.parent:
			self.unobserve( self.parent.getShape(), 'sizeChanged' )

		if self.child:
			self.unobserve( self.child.getShape(), 'sizeChanged' )


	def enableAutomaticJointScaling( self ):
		'''Tracks changes to sizes in the link shapes, and automatically "scales" the  joint (more accurately, the link points) such that the resized links will maintain the same relative distance from one another.  This feature is likely to be desirable whenever "growth" of OBJECT(MultiBody) objects is simulated. <P> This can be later disabled using METHOD(disable-automatic-joint-scaling).'''


		self.disableAutomaticJointScaling()
		if self.parent:
			self.observe( self.parent.getShape(), 'sizeChanged', 'rescaleParentLinkPoint' )

		self.observe( self.child.getShape(), 'sizeChanged', 'rescaleChildLinkPoint' )

	def enableChildrenReposition( self ):

		breve.breveInternalFunctionFinder.jointSetRepositionAll( self, self.jointPointer, 1 )

	def getChild( self ):
		'''Returns the object identified as the "child" ojbect.'''


		return self.child

	def getChildLinkPoint( self ):
		'''Returns the child link point.'''


		return self.clinkPoint

	def getForce( self ):
		'''Returns the force applied by this joint to maintain the desired relationships of the connected bodies.  In other words, the stress  force on the joint.   <P> The forces applied to connected bodies are generally equal and  opposite, so this value is computed as f1 + -f2, where f1 and f2 are the forces acting on the first and second bodies.'''


		if self.jointPointer:
			return breve.breveInternalFunctionFinder.jointGetForce( self, self.jointPointer )


	def getJointNormal( self ):
		'''Returns the joint's normal vector.'''


		return self.normalVector

	def getJointPointer( self ):
		'''Internal use only.'''


		return self.jointPointer

	def getJointVelocity( self ):
		'''Returns the scalar velocity of the joint.  This is only useful for 1-DOF joints (prismatic and revolute).  For higher-DOF joints, use METHOD(get-joint-velocity-vector).'''


		return breve.breveInternalFunctionFinder.jointGetVelocity( self, self.jointPointer ).x

	def getJointVelocityVector( self ):
		'''Returns the vector velocity of the joint.  For 1-DOF joints (prismatic, revolute), only the X-component is valid.  For 2-DOF joints (universal), both the X- and the Y-components are used.  For a ball joint, all three values are provided.'''


		return breve.breveInternalFunctionFinder.jointGetVelocity( self, self.jointPointer )

	def getMaxJointLimitVector( self ):
		'''Returns the maximum joint limit vector, if it has been set.'''


		return self.maxVector

	def getMinJointLimitVector( self ):
		'''Returns the minimum joint limit vector, if it has been set.'''


		return self.minVector

	def getParent( self ):
		'''Returns the object identified as the "parent" ojbect.'''


		return self.parent

	def getParentLinkPoint( self ):
		'''Returns the child link point.'''


		return self.plinkPoint

	def getStrengthHardLimit( self ):
		'''Returns the "hard" limit of the joint--the maximum torque that you are allowed to put on a single joint.'''


		if ( not self.child ):
			return 0

		return ( self.child.getMass() * 500 )

	def getStrengthLimit( self ):
		'''Returns the user set strength limit of the joint.'''


		return self.strengthLimit

	def getTorque( self ):
		'''Returns the torque applied by this joint to maintain the desired relationships of the connected bodies.  In other words, the stress  torque on the joint. <P> The torques applied to connected bodies are generally equal and  opposite, so this value is computed as t1 + -t2, where t1 and t2 are the torques acting on the first and second bodies.'''


		if self.jointPointer:
			return breve.breveInternalFunctionFinder.jointGetTorque( self, self.jointPointer )


	def init( self ):

		self.normalVector
		self.relativeRotation = breve.matrix(  1, 0, 0, 0, 1, 0, 0, 0, 1 )

	def rescaleChildLinkPoint( self ):
		'''Used internally to rescale link points when joint scaling is enabled.'''

		scale = breve.vector()

		if ( not self.child ):
			return

		scale = self.child.getShape().getLastScale()
		self.clinkPoint.x = ( self.clinkPoint.x * scale.x )
		self.clinkPoint.y = ( self.clinkPoint.y * scale.y )
		self.clinkPoint.z = ( self.clinkPoint.z * scale.z )
		self.setChildLinkPoint( self.clinkPoint )

	def rescaleParentLinkPoint( self ):
		'''Used internally to rescale link points when joint scaling is enabled.'''

		scale = breve.vector()

		if ( not self.parent ):
			return

		scale = self.parent.getShape().getLastScale()
		self.plinkPoint.x = ( self.plinkPoint.x * scale.x )
		self.plinkPoint.y = ( self.plinkPoint.y * scale.y )
		self.plinkPoint.z = ( self.plinkPoint.z * scale.z )
		self.setParentLinkPoint( self.plinkPoint )

	def setCfm( self, cfmValue ):
		'''Sets the joint's constraint force mixing parameter.  See the ODE physics engine documentation for more details.'''


		breve.breveInternalFunctionFinder.jointSetCFM( self, self.jointPointer, cfmValue )

	def setChildLinkPoint( self, linkPoint ):
		'''Dynamically sets the child's link point for this joint.  This method can be called over the course of the simulation to change the way the joint works. <p> If the Joint is not linked, this call has no effect.'''


		self.clinkPoint = linkPoint
		if self.jointPointer:
			breve.breveInternalFunctionFinder.jointSetLinkPoints( self, self.jointPointer, self.plinkPoint, self.clinkPoint, self.relativeRotation )


	def setDoubleSpring( self, theStrength, theMax, theMin ):
		'''Enable this joint's spring to take effect when the rotation is above theMax or below theMin.  When the position of the joint is between theMin and theMax, the spring applies no force to the joint.  When the position goes out of this range a force is applied according to the strength of the joint (theStrength) and distance to the "natural" range. <p> To revert to the default behavior of no spring acting on the joint, use the method METHOD(remove-spring).'''


		if ( not self.jointPointer ):
			raise Exception( '''attempting to set spring of uninitialized Joint object.''' )


		self.springMax = theMax
		self.springMin = theMin
		self.springStrength = theStrength
		breve.breveInternalFunctionFinder.jointSetSpring( self, self.jointPointer, theStrength, theMin, theMax )

	def setErp( self, erpValue ):
		'''Sets the joint's error correction parameter.  See the ODE physics engine documentation for more details.'''


		breve.breveInternalFunctionFinder.jointSetERP( self, self.jointPointer, erpValue )

	def setJointDamping( self, dampingValue ):
		'''Sets the joint damping--the friction of the joint.  '''


		breve.breveInternalFunctionFinder.jointSetDamping( self, self.jointPointer, dampingValue )

	def setJointLimitVectors( self, minV, maxV ):
		'''A convenience method for setting joint limits for any kind of  joint.  For 1-DOF joints (prismatic, revolute) only the X values from the vectors are used.  For 2-DOF joints (universal), both X and Y values are used.  For 3-DOF joints, all the  values are used. <P> Note that this method is just an alternative to the set-joint-limit methods which exist in all of the joints which support limits.'''


		self.minVector = minV
		self.maxVector = maxV
		if self.jointPointer:
			breve.breveInternalFunctionFinder.jointSetLimits( self, self.jointPointer, minV, maxV )


	def setJointNormal( self, normal ):
		'''If this type of joint supports it this method sets the  normal vector for the motion for the joint.  The precise meaning  of the normal vector depends on the exact type of joint.  See each joint's link method documentation for more details.'''


		self.normalVector = normal
		if self.jointPointer:
			breve.breveInternalFunctionFinder.jointSetNormal( self, self.jointPointer, self.normalVector )


	def setJointTorqueVector( self, torqueVector ):
		'''Applies torque on one or more axes.  This method can be used when setting the torque of ball or universal joints.'''


		if self.jointPointer:
			breve.breveInternalFunctionFinder.jointApplyForce( self, self.jointPointer, torqueVector )


	def setJointVelocity( self, newVelocity ):
		'''Sets the joint velocity to newVelocity.'''


		if self.jointPointer:
			breve.breveInternalFunctionFinder.jointSetVelocity( self, self.jointPointer, breve.vector( newVelocity, 0, 0 ) )


	def setParentLinkPoint( self, linkPoint ):
		'''Dynamically sets the parent's link point for this joint.  This method can be called over the course of the simulation to change the way the joint works. <p> If the Joint is not linked, this call has no effect.'''


		self.plinkPoint = linkPoint
		if self.jointPointer:
			breve.breveInternalFunctionFinder.jointSetLinkPoints( self, self.jointPointer, self.plinkPoint, self.clinkPoint, self.relativeRotation )


	def setRelativeRotation( self, theAxis, theAngle ):
		'''To avoid undesirable "jumping" behavior, this method should be called  <b>before</b> linking the joint. <p> Sets the relative rotation between the child and parent OBJECT(Link) to newRotation.  This rotation specifies the rotation of the child link when the joint is "relaxed" or at its natural position. <p> Changing the relative rotation of the joint fundamentally changes the way the joint works, and should not be used as a "normal" joint motion: the relative rotation may be changed dynamically over the course of  the simulation, though it <b>may cause unexpected and unrealistic  behavior</b> if the change is not made gradually, or if the bodies  involved are in contact with other bodies at the time of the change.'''


		self.relativeRotation = breve.breveInternalFunctionFinder.rotationMatrix( self, theAxis, theAngle )
		if self.jointPointer:
			breve.breveInternalFunctionFinder.jointSetLinkPoints( self, self.jointPointer, self.plinkPoint, self.clinkPoint, self.relativeRotation )


	def setRelativeRotationMatrix( self, newRotation ):
		'''To avoid undesirable "jumping" behavior, this method should be called  <b>before</b> linking the joint. <p> Sets the relative rotation between the child and parent OBJECT(Link) to newRotation.  This rotation specifies the rotation of the child link when the joint is "relaxed" or at its natural position. <p> The method METHOD(set-relative-rotation) is somewhat easier to use, since it does not require the actual rotation matrix. <p> Changing the relative rotation of the joint fundamentally changes the way the joint works, and should not be used as a "normal" joint motion: the relative rotation may be changed dynamically over the course of  the simulation, though it <b>may cause unexpected and unrealistic  behavior</b> if the change is not made gradually, or if the bodies  involved are in contact with other bodies at the time of the change.'''


		self.relativeRotation = newRotation
		if self.jointPointer:
			breve.breveInternalFunctionFinder.jointSetLinkPoints( self, self.jointPointer, self.plinkPoint, self.clinkPoint, self.relativeRotation )


	def setStrengthLimit( self, newLimit ):
		'''Sets the maximum torque to be used by this joint when attempting  to attain the target speed.'''


		if ( not self.jointPointer ):
			return

		if ( newLimit > self.getStrengthHardLimit() ):
			newLimit = self.getStrengthHardLimit()

		if ( newLimit < 0 ):
			newLimit = 0.000000

		self.strengthLimit = newLimit
		breve.breveInternalFunctionFinder.jointSetMaxStrength( self, self.jointPointer, newLimit )


breve.Joint = Joint
class PrismaticJoint( breve.Joint ):
	'''This subclass of OBJECT(Joint) is used to link two OBJECT(Link) objects together using a sliding linear joint.   <P> A retractable TV antenna is an example of an object that uses multiple prismatic joints. <P> <CENTER><IMG SRC="PrismaticJoint.jpg" BORDER=1></CENTER>'''

	def __init__( self ):
		breve.Joint.__init__( self )

	def dearchive( self ):

		if ( self.parent or self.child ):
			self.link( self.parent, self.child, self.normalVector, self.plinkPoint, self.clinkPoint )

		return breve.Joint.dearchive( self )

	def getJointPosition( self ):
		'''Returns a double indicating how far this joint is from its natural position (which corresponds to a value of 0).'''


		return breve.breveInternalFunctionFinder.jointGetPosition( self, self.jointPointer ).x

	def link( self, theNormal, parentPoint, childPoint, childLink, parentLink = 0.000000, currentRotation = 0.000000 ):
		'''Creates a prismatic (sliding) joint between parentLink and childLink.   The joint will be locoated at parentPoint on the parent's body, and at childPoint on the child's body.  The joint will slide along the axis  theNormal. <P> If parentLink is the value 0, then the joint will be attached to a  fixed point in the world, specified with parentPoint.'''

		cpointer = None
		ppointer = None

		if parentLink:
			ppointer = parentLink.getLinkPointer()

		if childLink:
			cpointer = childLink.getLinkPointer()

		if ( parentLink and ( not ppointer ) ):
			raise Exception( '''attempting to link uninitialized Link object %s.''' % (  parentLink ) )


		if ( childLink and ( not cpointer ) ):
			raise Exception( '''attempting to link uninitialized Link object %s.''' % (  childLink ) )


		self.jointPointer = breve.breveInternalFunctionFinder.jointLinkPrismatic( self, ppointer, cpointer, theNormal, parentPoint, childPoint, self.relativeRotation, currentRotation )
		self.normalVector = theNormal
		self.plinkPoint = parentPoint
		self.clinkPoint = childPoint
		self.parent = parentLink
		self.child = childLink
		self.addDependency( self.child )
		self.addDependency( self.parent )

	def setJointForce( self, forceValue ):
		'''Applies a force of forceValue to this joint. '''


		breve.breveInternalFunctionFinder.jointApplyForce( self, self.jointPointer, breve.vector( forceValue, 0, 0 ) )

	def setJointLimits( self, min1, max1 ):
		'''Sets the minimum and maximum allowed positions for this  joint. '''


		breve.breveInternalFunctionFinder.jointSetLimits( self, self.jointPointer, breve.vector( min1, 0, 0 ), breve.vector( max1, 0, 0 ) )


breve.PrismaticJoint = PrismaticJoint
class RevoluteJoint( breve.Joint ):
	'''This subclass of OBJECT(Joint) is used to link two OBJECT(Link) objects together using a rotating joint.   <P> RevoluteJoints rotate on a single axis, like a hinge.  The knee is another  example of a RevoluteJoint--it can bend back-and-forth, but it cannot twist  or bend side-to-side. <P> <CENTER><IMG SRC="RevoluteJoint.jpg" BORDER=1></CENTER>'''

	def __init__( self ):
		breve.Joint.__init__( self )

	def dearchive( self ):

		if ( self.parent or self.child ):
			self.link( self.parent, self.child, self.normalVector, self.plinkPoint, self.clinkPoint )

		return breve.Joint.dearchive( self )

	def getJointAngle( self ):
		'''Returns a double indicating how the angle of this joint from its natural position (which corresponds to a value of 0).'''


		return breve.breveInternalFunctionFinder.jointGetPosition( self, self.jointPointer ).x

	def getJointPosition( self ):

		return self.getJointAngle()

	def link( self, theNormal, parentPoint, childPoint, childLink, parentLink = 0.000000, currentRotation = 0.000000 ):
		'''Creates a revolute (rotating) joint between parentLink and childLink.   The joint will be lcoated at parentPoint on the parent's body, and at childPoint on the child's body.  The joint will rotate along the axis  theNormal. <P> If parentLink is the value 0, then the joint will be attached to a  fixed point in the world, specified with parentPoint.'''

		cpointer = None
		ppointer = None

		if parentLink:
			ppointer = parentLink.getLinkPointer()

		if childLink:
			cpointer = childLink.getLinkPointer()

		if ( parentLink and ( not ppointer ) ):
			raise Exception( '''attempting to link uninitialized Link object %s.''' % (  parentLink ) )


		if ( childLink and ( not cpointer ) ):
			raise Exception( '''attempting to link uninitialized Link object %s.''' % (  childLink ) )


		if self.jointPointer:
			raise Exception( '''attempt to link previously linked Joint object.''' )


		self.jointPointer = breve.breveInternalFunctionFinder.jointLinkRevolute( self, ppointer, cpointer, theNormal, parentPoint, childPoint, self.relativeRotation, currentRotation )
		self.normalVector = theNormal
		self.plinkPoint = parentPoint
		self.clinkPoint = childPoint
		self.parent = parentLink
		self.child = childLink
		self.addDependency( self.child )
		self.addDependency( self.parent )

	def setJointLimits( self, min1, max1 ):
		'''Sets the minimum and maximum allowed rotations (in radians)  for this joint. '''


		breve.breveInternalFunctionFinder.jointSetLimits( self, self.jointPointer, breve.vector( min1, 0, 0 ), breve.vector( max1, 0, 0 ) )

	def setJointTorque( self, torqueValue ):
		'''Applies a torque of torqueValue to the revolute joint. '''


		breve.breveInternalFunctionFinder.jointApplyForce( self, self.jointPointer, breve.vector( torqueValue, 0, 0 ) )


breve.RevoluteJoint = RevoluteJoint
class FixedJoint( breve.Joint ):
	'''This subclass of OBJECT(Joint) is used to link two OBJECT(Link) objects together using a static, or fixed, joint. <P> <CENTER><IMG SRC="FixedJoint.jpg" BORDER=1></CENTER> <P> <b>Fixed joints do not currently support a relative rotation the way other joint types do.</b>  I'm so sorry.  '''

	def __init__( self ):
		breve.Joint.__init__( self )

	def dearchive( self ):

		print '''dearchiving for %s and %s''' % (  self.parent, self.child )
		if ( self.parent or self.child ):
			self.link( self.parent, self.child, self.plinkPoint, self.clinkPoint )

		return breve.Joint.dearchive( self )

	def link( self, childLink, parentLink = 0.000000, parentPoint = breve.vector( 0.000000, 0.000000, 0.000000 ), childPoint = breve.vector( 0.000000, 0.000000, 0.000000 ), currentRotation = 0.000000 ):
		'''Creates a fixed joint between parentLink and childLink.   The joint will be located at parentPoint on the parent's body, and at childPoint on the child's body.   <P> Unfortunately, FixedJoints do not use the relative joint orientation that can be set in OBJECT(Joint).  This means that the rotation of the  child is always the same as the rotation of the parent. <P> If parentLink is the value 0, then the joint will be attached to a  fixed point in the world, specified with parentPoint.'''

		cpointer = None
		ppointer = None

		if parentLink:
			ppointer = parentLink.getLinkPointer()

		if childLink:
			cpointer = childLink.getLinkPointer()

		if ( parentLink and ( not ppointer ) ):
			raise Exception( '''attempting to link uninitialized Link object %s.''' % (  parentLink ) )


		if ( childLink and ( not cpointer ) ):
			raise Exception( '''attempting to link uninitialized Link object %s.''' % (  childLink ) )


		self.jointPointer = breve.breveInternalFunctionFinder.jointLinkStatic( self, ppointer, cpointer, breve.vector( 0, 0, 0 ), parentPoint, childPoint, self.relativeRotation, currentRotation )
		self.normalVector = breve.vector( 1, 0, 0 )
		self.plinkPoint = parentPoint
		self.clinkPoint = childPoint
		self.parent = parentLink
		self.child = childLink
		self.addDependency( self.child )
		self.addDependency( self.parent )


breve.FixedJoint = FixedJoint
class UniversalJoint( breve.Joint ):
	'''This subclass of OBJECT(Joint) is used to link two OBJECT(Link) objects together using a universal joint.   <P> A universal joint has two degrees of freedom.  It can rotate "up-and-down" and "side-to-side", but cannot  "twist".  Your wrist is basically a universal joint--your hand cannot rotate without the rest of your arm. <P> <CENTER><IMG SRC="UniversalJoint.jpg" BORDER=1></CENTER>'''

	def __init__( self ):
		breve.Joint.__init__( self )

	def dearchive( self ):

		if ( self.parent or self.child ):
			self.link( self.parent, self.child, self.normalVector, self.plinkPoint, self.clinkPoint )

		return breve.Joint.dearchive( self )

	def getJointAngles( self ):
		'''Returns a vector indicating how the angle of this joint is from its natural position (which corresponds to a values of 0 on both axes).  Only the X- and Y- components of the vector are valid for UniversalJoints.'''


		return breve.breveInternalFunctionFinder.jointGetPosition( self, self.jointPointer )

	def link( self, normal, parentPoint, childPoint, childLink, parentLink = 0.000000, currentRotation = 0.000000 ):
		'''Creates a universal (rotation on two axes) joint between parentLink and childLink.   The joint will be lcoated at parentPoint on the parent's body, and at childPoint on the child's body.   <P> Universal joints have two rotation axes, both of which are perpendicular to the  parentPoint.  Think about the forearm linking to the hand at the wrist--the axes of rotation of the wrist are both perpendicular to the line from the center of the forearm (which represents the parentPoint).  The normalVector specifies <i>one</i> of the rotation axes, while the other is calculated automatically (perpendicular to  both parentPoint and normalVector). <P> If parentLink is the value 0, then the joint will be attached to a  fixed point in the world, specified with parentPoint.'''

		cpointer = None
		ppointer = None

		if parentLink:
			ppointer = parentLink.getLinkPointer()

		if childLink:
			cpointer = childLink.getLinkPointer()

		if ( parentLink and ( not ppointer ) ):
			raise Exception( '''attempting to link uninitialized Link object %s.''' % (  parentLink ) )


		if ( childLink and ( not cpointer ) ):
			raise Exception( '''attempting to link uninitialized Link object %s.''' % (  childLink ) )


		if self.jointPointer:
			raise Exception( '''attempt to link previously linked Joint object.''' )


		self.jointPointer = breve.breveInternalFunctionFinder.jointLinkUniversal( self, ppointer, cpointer, normal, parentPoint, childPoint, self.relativeRotation, currentRotation )
		self.normalVector = normal
		self.plinkPoint = parentPoint
		self.clinkPoint = childPoint
		self.parent = parentLink
		self.child = childLink
		self.addDependency( self.child )
		self.addDependency( self.parent )

	def setJointLimits( self, min1, max1, min2, max2 ):
		'''Sets the minimum and maximum allowed rotations (in radians)  for this joint on both of the joint's axes.  '''


		breve.breveInternalFunctionFinder.jointSetLimits( self, self.jointPointer, breve.vector( min1, min2, 0 ), breve.vector( max1, max2, 0 ) )

	def setJointVelocity( self, a1velocity, a2velocity ):
		'''Sets the desired joint velocity on axis-1 to a1velocity and on axis-2 to a2velocity.'''


		if self.jointPointer:
			breve.breveInternalFunctionFinder.jointSetVelocity( self, self.jointPointer, breve.vector( a1velocity, a2velocity, 0 ) )



breve.UniversalJoint = UniversalJoint
class BallJoint( breve.Joint ):
	'''This subclass of OBJECT(Joint) is used to link two OBJECT(Link) objects together using a ball joint. <p> A BallJoint has three degrees of freedom.  It can tilt "up-and-down" and "side-to-side", and can "twist".  Your neck is basically a ball joint--your head tilt  up-and-down, side-to-side, and can twist (although not all the  way around--if your head turns all the way around, please consult  the user manual for repair). <P> <CENTER><IMG SRC="BallJoint.jpg" BORDER=1></CENTER> <P> This joint uses a scheme of 3 Euler angles to represent the  relative rotation between two objects.  The Z-axis of rotation is automatically  derived from the parent link point, meaning that the Z-axis rotation is always  a "twist" relative to the link point.  The X-axis of rotation is specified by  the user with the normal argument.  The Y-axis is automatically derived from the  other two. <P> Because of a limitation of the physics engine, the Y-axis rotation is limited  to a range of approximately +/- 80 degrees of rotation. <P> If versions of breve prior to 2.3, there was no normal argument because the  axes of rotation were automatically defined.  The argument is therefore optional for backwards compatibility <b>only</b>, and should <b>always</b> be specified when writing new code.'''

	def __init__( self ):
		breve.Joint.__init__( self )

	def dearchive( self ):

		if ( self.parent or self.child ):
			self.link( self.parent, self.child, self.plinkPoint, self.clinkPoint )

		return breve.Joint.dearchive( self )

	def getJointAngles( self ):
		'''Returns a vector indicating how the angle of this joint is from its natural position (which corresponds to a values of 0 on all axes).  '''


		return breve.breveInternalFunctionFinder.jointGetPosition( self, self.jointPointer )

	def link( self, parentPoint, childPoint, childLink, parentLink = 0.000000, normal = breve.vector( 1.000000, 0.000000, 0.000000 ), currentRotation = 0.000000 ):
		'''Creates a ball (rotation on three axes) joint between parentLink and childLink.   The joint will be lcoated at parentPoint on the parent's body, and at childPoint on the child's body.   <P> If parentLink is the value 0, then the joint will be attached to a  fixed point in the world, specified with parentPoint.'''

		cpointer = None
		ppointer = None

		if parentLink:
			ppointer = parentLink.getLinkPointer()

		if childLink:
			cpointer = childLink.getLinkPointer()

		if ( childLink and ( not cpointer ) ):
			raise Exception( '''attempting to link uninitialized Link object %s.''' % (  childLink ) )


		if self.jointPointer:
			raise Exception( '''attempt to link previously linked Joint object.''' )


		self.jointPointer = breve.breveInternalFunctionFinder.jointLinkBall( self, ppointer, cpointer, normal, parentPoint, childPoint, self.relativeRotation, currentRotation )
		self.normalVector = normal
		self.plinkPoint = parentPoint
		self.clinkPoint = childPoint
		self.parent = parentLink
		self.child = childLink
		self.addDependency( self.child )
		self.addDependency( self.parent )

	def setJointLimits( self, min1, max1, min2, max2, min3, max3 ):
		'''Sets the minimum and maximum allowed rotations (in radians)  for this joint on all three of the joint's axes.  '''


		if self.jointPointer:
			breve.breveInternalFunctionFinder.jointSetLimits( self, self.jointPointer, breve.vector( min1, min2, min3 ), breve.vector( max1, max2, max3 ) )


	def setJointVelocity( self, rotationalVelocity ):
		'''Sets the desired joint velocity to rotationalVelocity.  As a ball joint allows rotation on three axes, rotationalVelocity is a vector.'''


		if self.jointPointer:
			breve.breveInternalFunctionFinder.jointSetVelocity( self, self.jointPointer, rotationalVelocity )



breve.BallJoint = BallJoint
# Add our newly created classes to the breve namespace

breve.Joints = Joint
breve.PrismaticJoints = PrismaticJoint
breve.RevoluteJoints = RevoluteJoint
breve.FixedJoints = FixedJoint
breve.UniversalJoints = UniversalJoint
breve.BallJoints = BallJoint



