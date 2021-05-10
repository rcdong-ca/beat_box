This is our solution to the Beat box assignment for our embedded programming course.
<br />
The task was to create an application that plays a drum-beat on the Beagle bone using the Zen cape audio output and its joystick for input.
The audio is generated realtime using the ALSA API. There are different drum beat modes supported in this application <br />
 <br />
These sounds are from <b>Freesound.org, by menegass, used under Creative Commons License</b>  <br />
1. No drum beat<br />
2. Rock Beat<br />
3. Our custom drum beat<br />
<br />
The Joystick controls:<br />
1. Press in to cycle through our drum beat modes.<br />
2.Up increases the volumne by 5, while down deceases the volume by 5 (Min: 40, Max: 300)<br />
3.Left decreases the BPM by 5, Right increases the BPM my 5 (Min: 0, Max: 100)<br />
<br />
The accelorometer is also another way to generate sound with the Beagle Bone. It is able to detect significant accelearations in the 3 axis X,Y,Z, which is what allows
sound to be generating with movement.
By moving the beagle bone vertically, it will generate the base drum sound. Moving the Beagle Bone in the X Axis will play the snare while, Yaxis will play the hi-hat.
<br />
<br />


Node.js Web interface is also accompanied to help manage the controls in a more visual way and also tell you the state of the beagle bone too. <br />
Both the application and node server is intended to run on the target.
 <br />
![image](https://user-images.githubusercontent.com/52853478/117725639-44a9f100-b19a-11eb-8828-af9b58e452f0.png)

 
