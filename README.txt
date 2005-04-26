                      The breve Simulation Environment

                      http://www.spiderland.org/breve
                        release 2.2 - 04.27.2005
             command line version for Mac OS X, Linux and Windows
                         jklein@spiderland.org 

                           -= INTRODUCTION =-

breve is a simulation environment which allows users to define the behaviors 
and interactions of different types of agents in a simulated 3D world and 
observe the emergent behaviors.  breve is conceptually similar to packages 
such as Swarm and StarLogo, but simulates objects in a continuous 3D world 
with continuous time.  breve thus allows simulation involving realistic 
collision detection and physics.   

Agent behaviors are written in a simple object oriented language called 
"steve".  Users define an agent's behavior by specifying how the agent 
interacts with the world at each timestep and how the agent interacts when 
it collides with other agents. 

breve includes full collision detection and physical simulation of rigid 
bodies.  See the Walker and Gravity demo simulations for examples.   

For more detailed information about the breve environment, consult the 
documentation in the "docs" directory.

                             -= GETTING STARTED =- 

NOTE FOR WINDOWS USERS: You'll need to download the GLUT library in order
to run breve.  One source for this is: 
http://www.xmission.com/~nate/glut.html

The best way to get started with breve is to run some of the included demo 
simulations, which are found in the "demos" directory.  More complete 
information on programming and running simulations with breve is available 
in the documentation, which is found in the "docs" directory.

These are the directions to get started quickly with the command-line 
version of breve 2.2.

1) set the environment variable BREVE_CLASS_PATH to the full path of the
   included "lib/classes" directory.  For example, using the bash shell
   under Linux or Mac OS X:

       export BREVE_CLASS_PATH=<path to breve>/lib/classes

   Under windows, this might be something like:

       set BREVE_CLASS_PATH=C:\<path to breve>\lib\classes

2) run breve with one of the demo files: breve <simulation file>.
   For example:

       ./bin/breve ./demos/Gatherers.tz

   Or under Windows:

        \BIN\BREVE.EXE DEMOS\GATHERERS.TZ

3) press space bar to toggle the simulation, <esc> to exit.  Use the left
   mouse button to select objects and to rotate the simulation, left  
   mouse button + shift to zoom the camera and the right mouse button to 
   access the simulation menu (or contextual menus associated with individual
   objects).  F1 steps forward the simulation a single step.

For more information about what the simulations do and how they work, open
up the demo files in any text editor and read the comments at the top of 
the file.

                               -= FEEDBACK =-

This software is provided with the hope that it will be useful--the only way 
for me to make the software more useful is to hear your feedback.  You can 
send me mail directly at jk@spiderland.org.  Please let me know how you 
use the software and how it could be improved. 

If you have any cool or useful simulations or classes that should be included 
with future releases, mail them as well. 

If you would like to join the breve users mailing list, you can subscribe at 
http://lists.spiderland.org/mailman/listinfo/breve .

                                 -= LICENSE =-

The breve Simulation Environment                                           
Copyright (C) 2003, 2004 Jonathan Klein                        

This program is free software; you can redistribute it and/or modify       
it under the terms of the GNU General Public License as published by       
the Free Software Foundation; either version 2 of the License, or          
(at your option) any later version.                                        

This program is distributed in the hope that it will be useful,            
but WITHOUT ANY WARRANTY; without even the implied warranty of             
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              
GNU General Public License for more details.                               

You should have received a copy of the GNU General Public License          
along with this program; if not, write to the Free Software                
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  

--

Portions of this software use the Open Dynamics Engine library, under the
following license: 

Open Dynamics Engine
Copyright (c) 2001, 2002, Russell L. Smith.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

Neither the names of ODE's copyright owner nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

--

Portions of this software use the expat library, under the following license: 

Copyright (c) 1998, 1999, 2000 Thai Open Source Software Center Ltd

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
