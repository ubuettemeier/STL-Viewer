## STL-Viewer
simple STL File Viewer
<pre>usage: ./stlviewer  file, ...
example: ./stlviewer  STL_data/baby-tux_bin.STL</pre>
--- Entwicklungsumgebung ---

- Linux Ubuntu 20.04.1
- gcc c++1y
- OpenGL Version= 4.6 (Compatibility Profile) Mesa 21.0.3

<pre>
---- Keyboard shortcuts ----

h : this message
k : show special key's
t : draw triangles ON/OFF
l : draw lines ON/OFF
p : draw points ON/OFF
a : draw axis, Max, planes ON/OFF
f : Model einpassen (fit in)
v : Vorderansicht XY-plane
d : Draufsicht XZ-plane
s : Seitenansicht von links YZ-plane
o : optimiere Normal-Vektoren
c : draw Flächenrückseite (back face) on/off

0 : Light 0 on/off
1 : Light 1 on/off
</pre>
<pre>
---- special key's ----

            +|- : zoom
        →|←|↑|↓ : rotation 15°
Shift + →|←|↑|↓ : rotation 90°
 Strg + →|←|↑|↓ : move

       mouse left button : rotation
             mouse wheel : zoom
Strg + mouse left button : move
</pre>
<hr></hr>

![alt](README.png)
