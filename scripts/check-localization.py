"""Audit UI coverage and generated translation reproducibility, without running a game."""
from pathlib import Path
import json,re,runpy
root=Path(__file__).resolve().parents[1]
namespace=runpy.run_path(str(root/'scripts/generate-translations.py'))
keys=set(namespace['strings'])
technical={'F','Ctrl+','DM','DPS','Enter','Insert','.tmp','Segoe UI','Overlay','Corner',
 'ScalePercent','Opacity','Width','MaxRows','ShowDamage','ShowPercent','ShowDps',
 'ShowParticipation','ShowComparison','ShowKey','HistoryKey','SettingsKey','MoveKey','ShowBadge','AutoShow','ArchiveHunts',
 'ChatOpenKey','PositionX','PositionY','iceborne_damage_meter.ini',
 'iceborne_damage_meter_history.dat','iceborne_damage_meter_last_hunt.txt','iceborne_damage_meter_hunts.bin','wt, ccs=UTF-8'}
formats=namespace['tokens']
for name in ['panel.hpp','panel_overlay.inl','overlay.cpp','history_store.cpp','share_chat.hpp','quests.hpp','weapons.hpp']:
 text=(root/'src'/name).read_text(encoding='utf-8-sig')
 for match in re.finditer(r'L"((?:[^"\\]|\\.)*)"',text):
  value=json.loads('"'+match[1]+'"')
  if value in technical:continue
  remaining=re.sub(r'\b(?:DPS|s)\b','',formats.sub('',value))
  if not re.search(r'[A-Za-záéíóúñÁÉÍÓÚÑ]',remaining):continue
  assert value in keys,(name,value,'translation missing')
  if name!='weapons.hpp':assert text[max(0,match.start()-3):match.start()]=='tr(',(name,value,'not translated at call site')
print('UI literal coverage passed; configuration keys, numeric formats and player data remain separate.')
