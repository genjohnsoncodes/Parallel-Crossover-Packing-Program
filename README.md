# The rectangular strip packing problem solved with PCPP

The Parallel Crossover Packing Program (PCPP) is a program which seeks to find optimal solutions to the RSPP problem through the use of an evolutionary algorithm. The evolutionary algorithm PCPP employs the use of a newly created crossover operator hereby named Parallel Crossover. PCPP also uses biased mutation and generational populations in its evolutionary algorithm.

The rectangular strip packing problem (RSPP) is one of a variety of cutting-stock problems. Given a set of n rectangles, each with dimensions wi x hi  and a sheet with a set width and infinite height, the objective is to pack all the rectangles, without overlap, as to minimize the overall total height of the sheet on which they are packed.

To use PCPP run the executable with a file containing the sheet size, number of rectangles, and the set of rectangles which will be packed (-i followed by file name). A user can also change the mutation rate (-m followed by the desired mutation rate), population size, (-p followed by desired population size), and if the rectangles can be rotated or not (-r followed by 1 or 0). An example follows:
.\BinPackTest -i ..\\datasets\\c4p3.txt -o logfile.txt -p 130 -m 6

Default settings:
Mutations = 2
Population size = 100
Rotation = 1 (on)

This project was created for an Evolutionary Computation class at St. Cloud State University in collaboration with Jordan Markwardt and Ben Jacobs. Special thanks to Jukka Jylänki for their rectangular bin packing program (Jylänki 2010). PCPP used this open source code to pack the rectangles for each dataset. By submitting this program to public domain, the creators of PCPP were able to devote their time to creating the EA methods PCPP employs.
