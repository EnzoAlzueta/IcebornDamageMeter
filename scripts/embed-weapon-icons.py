"""Embed the existing game icon strip; no decoder or Python is needed at runtime."""
from pathlib import Path
import json
root=Path(__file__).resolve().parents[1]
data=(root/'assets/game_weapon_icons.png').read_bytes()
assert data[:8]==b'\x89PNG\r\n\x1a\n'
lines=['#pragma once','#include <array>','namespace meter {',
 '// Original game artwork: see assets/game_weapon_icons.json and Game-artwork-NOTICE.txt.',
 'inline constexpr std::array<unsigned char,'+str(len(data))+'> GameWeaponIconsPng{{']
for i in range(0,len(data),24):lines.append('    '+','.join(str(v) for v in data[i:i+24])+',')
columns=json.loads((root/'assets/game_weapon_icons.json').read_text(encoding='utf-8'))['nativeWeaponToAtlasColumn']
assert sorted(columns)==list(range(14))
lines+=['}};','inline constexpr std::array<unsigned,14> WeaponAtlasColumns{{'+','.join(map(str,columns))+'}};','}']
(root/'src/game_weapon_icons.hpp').write_text('\n'.join(lines)+'\n',encoding='utf-8')
print('Embedded',len(data),'bytes of original weapon icons.')
