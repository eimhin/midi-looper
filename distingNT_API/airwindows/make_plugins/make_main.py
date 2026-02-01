'''
MIT License

Copyright (c) 2025 Expert Sleepers Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
'''

import os

path = '../airwindows/plugins/MacAU'
files = os.listdir( path )
files = [ f for f in files if not os.path.isfile( os.path.join( path, f ) ) ]
files.sort()

try:
	os.mkdir( 'int' )
except FileExistsError:
	pass

for f in files:
	n = f;
	if f == 'Point':
		n = 'Poynt'
	with open( f'int/{f}.cpp', 'w' ) as F:
		F.write( '#include <iostream>\n' )
		F.write( f'#include "{path}/{f}/{n}.cpp"\n' )
		
		F.write( 'int main()\n' )
		F.write( '{\n' )
		F.write( '\tAudioUnit component;\n' )
		
		F.write( '\t{\n' )
		F.write( f'\t\t{n} p( component );\n' )
		F.write( '\t\tp.dumpParameters();\n' )
		F.write( '\t}\n' )
		
		F.write( '\treturn 0;\n' )
		F.write( '}\n' )
