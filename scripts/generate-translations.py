"""Validate and compile offline UI translations. No runtime files/network needed."""
from pathlib import Path
import json,re
root=Path(__file__).resolve().parents[1]
data=json.loads((root/'locales/strings.json').read_text(encoding='utf-8'))
strings=data['strings'];languages=data['languages']
assert len(languages)==12 and len(set(languages))==12
assert data['source_language']=='en' and languages[0]=='es'
tokens=re.compile(r'%(?:[+\- #0]*\d*(?:\.\d+)?(?:ll|l|z)?[diuoxXfFeEgGsc]|%)')
for key,values in strings.items():
 assert len(values)==len(languages),key
 for language,value in zip(languages,values):
  assert value and isinstance(value,str),(key,language)
  assert sorted(tokens.findall(key))==sorted(tokens.findall(value)),(key,language,'format mismatch')
def literal(text):return 'L'+json.dumps(text,ensure_ascii=False)
lines=['// Generated from locales/strings.json; original project translations, MIT.',
 '#pragma once','#include <array>','namespace meter {',
 'struct UiTranslation {const wchar_t* source;std::array<const wchar_t*,12> values;};',
 'inline constexpr UiTranslation UiTranslations[] = {']
for key in sorted(strings):
 lines.append('    {'+literal(key)+',{{'+','.join(literal(v) for v in strings[key])+'}}},')
lines+=['};','}']
(root/'src/translations_data.hpp').write_text('\n'.join(lines)+'\n',encoding='utf-8')
(root/'build').mkdir(exist_ok=True)
(root/'build/translation-keys.json').write_text(json.dumps(list(strings),ensure_ascii=False),encoding='utf-8')
print(f'{len(strings)} UI strings: English + {len(languages)} complete translations; format tokens verified.')
