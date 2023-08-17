#### Medusa (Greek: Μέδουσα, Chinese: 美杜莎) is a Gorgon in ancient Greek mythology, one of the three sisters of Gorgon

##### Focus on hard-hit areas such as processes, memory, threads, and kernel. Most of ARK has abandoned some functions that are not easy to maintain or add or are not stable enough in order to have enough functions.

##### Adding other more radical functions that ARK does not have becomes the main direction (virtual table hook, pointer replacement scanner, IOCTL check ,callback hook check , virtualization environment check , process link list broken check , driver link list broken check , memory-loading driver check , unknown memory scan , security dump , more and more tough The injection method , mock the anti-cheat or emulation checks....)

##### Guarantee the normal use and operation of some basic functions without loading PDB and drivers, and provide the maximum support that does not depend on the above two as much as possible

#### Completed (application layer):

##### ---Process thread module enumeration list

##### ---File memory code segment comparison (hook scan)

##### ---Application layer virtualization detection (including but not limited to various types of virtual machines and virtualization frameworks and drivers)

<h1 align="center">
	<img src="1.png" >
	<br>
	<br>
</h1>

#### Completed (kernel part):

##### ---Process disconnection check has a total of 4 checks and the results of the 4 checks are summarized. The third time is marked by ObjectTable "!NULL". The fourth time even if the ObjectTable is "!NULL", it is still added to the list and marked red. After the 4th check, most of the processes are disconnected. will be screened out

###### -----Application layer enumerates for the first time

###### -----The kernel calls ZwQuerySystemInformation enumeration for the second time

###### -----The kernel calls PsLookupProcessByProcessId to enumerate the process ID for the third check

###### -----The kernel calls PsLookupThreadByThreadId to enumerate the process ID for the fourth check

<h1 align="center">
	<img src="2.png" >
	<br>
	<br>
</h1>

###### The code is open source. If there is existing code, I will only sew and upload the release after the basic functions are complete.

