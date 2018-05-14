# SixTrackLib Concepts

## Main loop

The main loop maybe described with the following code


```python
for tt in range(nturns):
  for jj in range(len(elements)):
     for ii in range(len(nparticles)):
       if not_lost(particle[ii]):
         track_function=track[typeid(element[jj])]
         particle[ii]=track_function(particle[ii],element[jj])
```
where
- `particle[ii]` contains an array of coordinates for the particles `ii` that get updated as long as the particles is not lost
- `track[ ]` contains a set of tracking functions loosely representing a particular section of the accelerator (more specifically is a type of the integration step).
- `element[jj]` contains the type and the parameters for the tracking function of the element `jj`
- the loop runs for a predefined sequence of elements
