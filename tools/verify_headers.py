import os
import re

headers = [m.group(1) for m in re.finditer(r'#include\s+"([^"]+)"', open('include/FrameZero.h').read())]
missing = [h for h in headers if not os.path.exists(os.path.join('include', h))]
print('Total headers in FrameZero.h:', len(headers))
print('Missing:', missing if missing else 'None! All headers exist.')
