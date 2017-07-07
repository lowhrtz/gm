# -*- coding: utf-8 -*-
systemName = 'Osric'
subSystemName = 'OSRIC'
hasSeparateRacesAndClasses = True
attributes = [('STR', 'Strength', '3d6'),
              ('INT', 'Intelligence', '3d6'),
              ('WIS', 'Wisdom', '3d6'),
              ('DEX', 'Dexderity', '3d6'),
              ('CON', 'Constitution', '3d6'),
              ('CHA', 'Charisma', '3d6'),
#              ('COM','Comliness', '3d6'),
]
life = ['HP',]
alignment = ['Lawful Good', 'Neutral Good', 'Chaotic Good', 'Lawful Neutral', 'True Neutral', 'Chaotic Neutral', 'Lawful Evil', 'Neutral Evil', 'Chaotic Evil']
gender = ['Male','Female','NA']
economy = {
'gp':1,
'pp':5,
'ep':.5,
'sp':.1,
'cp':.01
}
restrictive_races = ['dwarf', 'half_orc']

bonuses = {
    'STR': [(3, '-3', '-1', '-35', '1', '0'),
            (5, '-2', '-1', '-25', '1', '0'),
            (7, '-1', '0', '-15', '1', '0'),
            (9, '0', '0', '0', '1-2', '1'),
            (11, '0', '0', '0', '1-2', '2'),
            (13, '0', '0', '+10', '1-2', '4'),
            (15, '0', '0', '+20', '1-2', '7'),
            (16, '0', '+1', '+35', '1-3', '10'),
            (17, '+1', '+1', '+50', '1-3', '13'),
            (18, '+1', '+2', '+75', '1-3', '16'),
            (18.5, '+1', '+3', '+100', '1-3', '20'),
            (18.75, '+2', '+3', '+125', '1-4', '25'),
            (18.9, '+2', '+4', '+150', '1-4', '30'),
            (18.99, '+2', '+5', '+200', '1-4(1 in 6)', '35'),
            (19, '+3', '+6', '+300', '1-5(1 in 6)', '40'),],
    'INT': [(3, '0'),
            (4, '0'),
            (5, '0'),
            (6, '0'),
            (7, '0'),
            (8, '1'),
            (9, '1'),
            (10, '2'),
            (11, '2'),
            (12, '3'),
            (13, '3'),
            (14, '4'),
            (15, '4'),
            (16, '5'),
            (17, '6'),
            (18, '7'),
            (19, '8'),],
    'WIS': [(3, '-3'),
            (4, '-2'),
            (5, '-1'),
            (6, '-1'),
            (7, '-1'),
            (8, '0'),
            (9, '0'),
            (10, '0'),
            (11, '0'),
            (12, '0'),
            (13, '0'),
            (14, '0'),
            (15, '+1'),
            (16, '+2'),
            (17, '+3'),
            (18, '+4'),
            (19, '+5'),],
    'DEX': [(3, '-3', '-3', '+4'),
            (4, '-2', '-2', '+3'),
            (5, '-1', '-1', '+2'),
            (6, '0', '0', '+1'),
            (7, '0', '0', '0'),
            (8, '0', '0', '0'),
            (9, '0', '0', '0'),
            (10, '0', '0', '0'),
            (11, '0', '0', '0'),
            (12, '0', '0', '0'),
            (13, '0', '0', '0'),
            (14, '0', '0', '0'),
            (15, '0', '0', '-1'),
            (16, '+1', '+1', '-2'),
            (17, '+2', '+2', '-3'),
            (18, '+3', '+3', '-4'),
            (19, '+3', '+3', '-4'),],
    'CON': [(3, '-2', '40', '35'),
            (4, '-1', '45', '40'),
            (5, '-1', '50', '45'),
            (6, '-1', '55', '50'),
            (7, '0', '60', '55'),
            (8, '0', '65', '60'),
            (9, '0', '70', '65'),
            (10, '0', '75', '70'),
            (11, '0', '80', '75'),
            (12, '0', '85', '80'),
            (13, '0', '90', '85'),
            (14, '0', '92', '88'),
            (15, '+1', '94', '91'),
            (16, '+2', '96', '95'),
            (17, '+2 (+3 for Warriors)', '98', '97'),
            (18, '+2 (+4 for Warriors)', '100', '99'),
            (19, '+2 (+5 for Warriors)', '100', '99'),],
    'CHA': [(3, '1', '-30', '-25'),
            (4, '1', '-25', '-20'),
            (5, '2', '-20', '-15'),
            (6, '2', '-15', '-10'),
            (7, '3', '-10', '-5'),
            (8, '3', '-5', '0'),
            (9, '4', '0', '0'),
            (10, '4', '0', '0'),
            (11, '4', '0', '0'),
            (12, '5', '0', '0'),
            (13, '5', '0', '+5'),
            (14, '6', '+5', '+10'),
            (15, '7', '+15', '+15'),
            (16, '8', '+20', '+25'),
            (17, '10', '+30', '+30'),
            (18, '15', '+40', '+35'),
            (19, '20', '+50', '+40'),],
}

def get_attribute_bonuses(attr_key, score):
    score = float(score)
    return_bonus = ()
    for row in bonuses[attr_key]:
        if score > row[0]:
            continue
        return_bonus = row
        break
    return tuple(bonus for bonus in return_bonus[1:])

def get_saves(level, attr_dict, class_dict, race_dict):
    saves_dict = {'Aimed_Magic_Items': [],
                  'Breath_Weapons': [],
                  'Death_Paralysis_Poison': [],
                  'Petrifaction_Polymorph': [],
                  'Spells': [],}
    if 'classes' in class_dict:
        for cl in class_dict['classes']:
            for meta_row in cl['Classes_meta']:
                if meta_row['Type'] == 'xp table' and meta_row['Level'] != 'each' and int(meta_row['Level']) == level:
                    for save in list(saves_dict.keys()):
                        saves_dict[save].append(meta_row[save])
        if race_dict['unique_id'] in restrictive_races:
            for save in list(saves_dict.keys()):
                saves_dict[save] = max(*tuple(int(l) for l in saves_dict[save]))
        else:
            for save in list(saves_dict.keys()):
                saves_dict[save] = min(*tuple(int(l) for l in saves_dict[save]))
    else:
        for meta_row in class_dict['Classes_meta']:
            if meta_row['Type'] == 'xp table' and meta_row['Level'] != 'each' and int(meta_row['Level']) == level:
                for save in list(saves_dict.keys()):
                    saves_dict[save] = meta_row[save]

    for meta_row in race_dict['Races_meta']:
        if meta_row['Type'] == 'ability' and meta_row['Subtype'] == 'save':
            modifier = meta_row['Modifier']
            modified = meta_row['Modified']
            num_modifier = ''
            for attr_name in list(attr_dict.keys()):
                if attr_name.lower() in modifier.lower():
                    num_modifier = modifier.lower().replace(attr_name.lower(), attr_dict[attr_name])
            #print num_modifier
            for mod in modified.split('/'):
                for save in list(saves_dict.keys()):
                    if mod.strip().replace(' ', '_') == save:
                        saves_dict[save] = eval(str(saves_dict[save]) + num_modifier)
                    elif mod.strip().replace(' ', '_') in save:
                        saves_dict[save] = str(saves_dict[save]) + ' (' + mod.strip() + ' ' + str(eval(str(saves_dict[save]) + num_modifier)) + ')'
                    elif save.replace('_', ' ') in mod:
                        mod_list = mod.split(':')
                        saves_dict[save] = str(saves_dict[save]) + ' (' + mod_list[1] + ' ' + str(eval(str(saves_dict[save]) + num_modifier)) + ')'
                    if len(str(saves_dict[save])) > 15:
                        saves_dict[save] = saves_dict[save].replace(' (', '<br />(')

    return saves_dict
