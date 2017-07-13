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
    'WIS': [(3, '-3', '0', '45'),
            (4, '-2', '0', '40'),
            (5, '-1', '0', '35'),
            (6, '-1', '0', '30'),
            (7, '-1', '0', '25'),
            (8, '0', '0', '20'),
            (9, '0', '0', '15'),
            (10, '0', '0', '10'),
            (11, '0', '0', '5'),
            (12, '0', '0', '1'),
            (13, '0', '1', '0'),
            (14, '0', '2', '0'),
            (15, '+1', '2/1', '0'),
            (16, '+2', '2/2', '0'),
            (17, '+3', '2/2/1', '0'),
            (18, '+4', '2/2/1/1', '0'),
            (19, '+5', '3/2/1/1', '0'),],
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

def calculate_ac(attr_dict, class_dict, race_dict, equipment_list):
    base_ac = 10
    dex_score = attr_dict['DEX']
    ac_bonus = get_attribute_bonuses('DEX', dex_score)[2]
    char_base_ac = base_ac + int(ac_bonus)

    armour_list = [e for e in equipment_list if e['unique_id'].startswith('armour_')]
    shield_list = [e for e in equipment_list if e['unique_id'].startswith('shield_')]

    useable_armour = []
    useable_shield = []

    if 'classes' in class_dict:
        ap_lists = []
        sp_lists = []
        for cl in class_dict['classes']:
            armour_permitted_list = [ap.strip() for ap in cl['Armour_Permitted'].split(',')]
            shield_permitted_list = [sp.strip() for sp in cl['Shield_Permitted'].split(',')]
            ap_lists.append(armour_permitted_list)
            sp_lists.append(shield_permitted_list)
        if race_dict['unique_id'] in restrictive_races:
            bucket = []
            all_any = True
            for ap_list in ap_lists:
                if 'None' in ap_list:
                    bucket = []
                    all_any = False
                    break
                elif 'Any' in ap_list:
                    continue
                else:
                    all_any = False
                    for ap in ap_list:
                        if ap not in bucket:
                            bucket.append(ap)
            if all_any:
                useable_armour = armour_list
            else:
                for a in armour_list:
                    if a['Name'] in bucket:
                        useable_armour.append(a)
            bucket = []
            all_any = True
            for sp_list in sp_lists:
                if 'None' in sp_list:
                    bucket = []
                    all_any = False
                    break
                elif 'Any' in ap_list:
                    continue
                else:
                    all_any = False
                    for sp in sp_list:
                        if sp not in sp_list:
                            bucket.append(sp)
            if all_any:
                useable_shield = shield_list
            else:
                for s in shield_list:
                    if s['Name'] in bucket:
                        useable_shield.append(s)
        else:
            bucket = []
            for ap_list in ap_lists:
                for ap in ap_list:
                    if ap not in bucket:
                        bucket.append(ap)

            if 'Any' in bucket:
                useable_armour = armour_list
            else:
                for a in armour_list:
                    if a['Name'] in bucket:
                        useable_armour.append(a)

            bucket = []
            for sp_list in sp_lists:
                for sp in sp_list:
                    if sp not in bucket:
                        bucket.append(sp)

            if 'Any' in bucket:
                useable_shield = shield_list
            else:
                for s in shield_list:
                    if s['Name'] in bucket:
                        useable_shield.append(s)
    else:
        armour_permitted_list = [ap.strip() for ap in class_dict['Armour_Permitted'].split(',')]
        shield_permitted_list = [sp.strip() for sp in class_dict['Shield_Permitted'].split(',')]
        if 'Any' in armour_permitted_list:
            useable_armour = armour_list
        elif 'None' in armour_permitted_list:
            useable_armour = []
        else:
            for a in armour_list:
                if a['Name'] in armour_permitted_list:
                    useable_armour.append(a)
        if 'Any' in shield_permitted_list:
            useable_shield = shield_list
        elif 'None' in shield_permitted_list:
            useable_shield = []
        else:
            for s in shield_list:
                if s['Name'] in shield_permitted_list:
                    useable_shield.append(s)

    best_armour = None
    for a in useable_armour:
        if not best_armour or a['AC_Effect'] < best_armour['AC_Effect']:
            best_armour = a
    best_shield = None
    for s in useable_shield:
        if not best_shield or s['AC_Effect'] < best_shield['AC_Effect']:
            best_shield = s

    if best_armour == None:
        best_armour = {'AC_Effect': 0}
    if best_shield == None:
        best_shield = {'AC_Effect': 0}
    return char_base_ac + best_armour['AC_Effect'] + best_shield['AC_Effect']

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

def get_tohit_row(level, class_dict, race_dict):
    tohit_list = []
    if 'classes' in class_dict:
        bucket = []
        for cl in class_dict['classes']:
            for row in cl['Classes_meta']:
                if row['Type'] == 'xp table' and row['Level'].isdigit() and int(row['Level']) == level:
                    tohit_tuple = (row['To_Hit_-10'],
                                   row['To_Hit_-9'],
                                   row['To_Hit_-8'],
                                   row['To_Hit_-7'],
                                   row['To_Hit_-6'],
                                   row['To_Hit_-5'],
                                   row['To_Hit_-4'],
                                   row['To_Hit_-3'],
                                   row['To_Hit_-2'],
                                   row['To_Hit_-1'],
                                   row['To_Hit_0'],
                                   row['To_Hit_1'],
                                   row['To_Hit_2'],
                                   row['To_Hit_3'],
                                   row['To_Hit_4'],
                                   row['To_Hit_5'],
                                   row['To_Hit_6'],
                                   row['To_Hit_7'],
                                   row['To_Hit_8'],
                                   row['To_Hit_9'],
                                   row['To_Hit_10'],
                                   )
                    bucket.append(tohit_tuple)
        for i in range(0, 21):
            items = (row[i] for row in bucket)
            if race_dict['unique_id'] in restrictive_races:
                tohit_list.append(str(max(*items)))
            else:
                tohit_list.append(str(min(*items)))

    else:
        for row in class_dict['Classes_meta']:
            if row['Type'] == 'xp table' and row['Level'].isdigit() and int(row['Level']) == level:
                tohit_list = [str(row['To_Hit_-10']),
                              str(row['To_Hit_-9']),
                              str(row['To_Hit_-8']),
                              str(row['To_Hit_-7']),
                              str(row['To_Hit_-6']),
                              str(row['To_Hit_-5']),
                              str(row['To_Hit_-4']),
                              str(row['To_Hit_-3']),
                              str(row['To_Hit_-2']),
                              str(row['To_Hit_-1']),
                              str(row['To_Hit_0']),
                              str(row['To_Hit_1']),
                              str(row['To_Hit_2']),
                              str(row['To_Hit_3']),
                              str(row['To_Hit_4']),
                              str(row['To_Hit_5']),
                              str(row['To_Hit_6']),
                              str(row['To_Hit_7']),
                              str(row['To_Hit_8']),
                              str(row['To_Hit_9']),
                              str(row['To_Hit_10']),
                              ]
    return tohit_list

def get_coinage_from_float(gp_decimal):
    bucket = int(gp_decimal * 100)
    coin_denominations = sorted(economy, key=lambda x: economy[x], reverse=True)
    return_dict = dict.fromkeys(coin_denominations, 0)
    for cd in coin_denominations:
        if economy[cd] <= 1:
            cd_bucket = 0
            while bucket >= economy[cd] * 100:
                bucket = bucket - int(economy[cd] * 100)
                cd_bucket = cd_bucket + 1
                return_dict[cd] = cd_bucket

    return return_dict

def calculate_movement(race_dict, class_dict, attr_dict, equipment_list):
    base_movement = race_dict['Movement_Rate']
    str_bonus = get_attribute_bonuses('STR', attr_dict['STR'])[2]
    max_movement = 120
    equipment_weight = 0
    wearable_armour = []
    for equip in equipment_list:
        if equip['Weight'] != 'N/A' and equip['Weight'] != '-':
            equip_weight = ''.join([c for c in equip['Weight'] if c.isdigit() or c == '/'])
            if equip_weight.find('/') != -1:
                equip_weight_tuple = equip_weight.split('/')
                numerator = int(equip_weight_tuple[0])
                denominator = int(equip_weight_tuple[1])
                equipment_weight += numerator / float(denominator)
            else:
                equipment_weight += float(equipment_weight)

        if equip['Max_Move_Rate'] != '' and equip['Max_Move_Rate'] != 'N/A':
            base_ac = calculate_ac(attr_dict, class_dict, race_dict, [])
            adj_ac = calculate_ac(attr_dict, class_dict, race_dict, [equip])
            if adj_ac < base_ac:
                wearable_armour.append(equip)
    best_armour = None
    for wa in wearable_armour:
        if not best_armour or wa['AC_Effect'] < best_armour['AC_Effect']:
            best_armour = wa
    if best_armour:
        max_move = ''.join([c for c in best_armour['Max_Move_Rate'] if c.isdigit()])
        if int(max_move) < max_movement:
            max_movement = int(max_move)

    if equipment_weight <= 35 + int(str_bonus):
        movement = (min(base_movement, max_movement), '+1 (for armour lighter than chain mail only)')
    elif equipment_weight <= 70 + int(str_bonus):
        movement = (min(base_movement - 30, max_movement), 'Normal bonuses apply')
    elif equipment_weight <= 105 + int(str_bonus):
        movement = (min(base_movement - 60, max_movement), 'No normal bonuses apply (but penalties do)')
    elif equipment_weight <= 150 + int(str_bonus):
        movement = (min(30, max_movement), 'No normal bonuses apply (but penalties do); -1 extra penalty')
    else:
        movement = (0, 'No movement possible')

    return movement

def get_spells_by_level(level, attr_dict, single_class_dict):
    primary_spell_string = 'Level_{}_Spells'
    secondary_spell_string = 'Level_{}_Spells_Secondary'
    primary = ''
    secondary = ''
    for row in single_class_dict['Classes_meta']:
        if row['Type'] == 'xp table' and row['Level'].isdigit() and int(row['Level']) == level and row['Casting_Level'] > 0:
            for i in range(1, 10):
                primary_key = primary_spell_string.format(i)
                secondary_key = secondary_spell_string.format(i)
                if row[primary_key] > 0:
                    primary_int = row[primary_key]
                    if i > 1:
                        primary += '/'
                    if single_class_dict['Category'] == 'priest':
                        bonus_spells = get_attribute_bonuses('WIS', attr_dict['WIS'])[1]
                        bonus_spells_list = bonus_spells.split('/')
                        if i - 1 < len(bonus_spells_list):
                            primary_int += int(bonus_spells_list[i - 1])
                    primary += str(primary_int)
                if row[secondary_key] > 0:
                    if i > 1:
                        secondary += '/'
                    secondary += str(row[secondary_key])
    return ( primary, secondary )

def get_turn_undead_row( level, single_class_dict ):
    tu_list = []
    col_string = 'Turn_Undead_Type_{}'
    for row in single_class_dict['Classes_meta']:
        if row['Type'] == 'xp table' and row['Level'].isdigit() and int( row['Level'] ) == level:
            for i in range( 1, 14 ):
                col = col_string.format(i)
                tu_list.append( row[col] )
    return tu_list

ta_col_list = ['Climb_Walls', 'Find_Traps', 'Hear_Noise', 'Hide_in_Shadows',
               'Move_Quietly', 'Open_Locks', 'Pick_Pockets', 'Read_Languages']
def get_thief_abilities_row( level, single_class_dict ):
    ta_list = []
    for row in single_class_dict['Classes_meta']:
        if row['Type'] == 'xp table' and row['Level'].isdigit() and int( row['Level'] ) == level:
            for ta_col in ta_col_list:
                ta_list.append( '{}%'.format( row[ta_col] ) )
    return ta_list

def get_class_abilities( level, attr_dict, single_class_dict ):
    spells_by_level = get_spells_by_level( level, attr_dict, single_class_dict )
    cl_ab = []
    if spells_by_level[0]:
        primary_spells = single_class_dict['Primary_Spell_List'].replace('_', ' ').title()
        cl_ab.append(('{} Spells by Level'.format(primary_spells), spells_by_level[0]))
        if spells_by_level[1]:
            secondary_spells = class_dict['Secondary_Spell_List'].replace('_', ' ').title()
            cl_ab.append(('{} Spells by Level'.format(secondary_spells), spells_by_level[1]))
    tu_list = get_turn_undead_row( level, single_class_dict )
    ta_list = get_thief_abilities_row( level, single_class_dict )
    for row in single_class_dict['Classes_meta']:
        if row['Type'] == 'ability' and level >= row['Level_Gained']:
            cl_ab.append((row['Ability'], row['Ability_Description']))
            if tu_list and 'turn' in row['Ability'].lower() and 'undead' in row['Ability'].lower():
                headers = [ 'Type {}'.format( i ) for i in range( 1, 14 ) ]
                cl_ab.append( ( '', ( headers, tu_list ) ) )
            if ta_list and 'thief abilities' in row['Ability'].lower():
                headers = [ h.replace( '_', ' ' ) for h in ta_col_list ]
                cl_ab.append( ( '', ( headers, ta_list ) ) )

    return cl_ab

def get_race_abilities( race_dict ):
    return_list = []
    subtype_list = [ 'combat', 'starting languages', 'infravision', 'misc' ]
    for row in race_dict['Races_meta']:
        if row['Type'] == 'ability' and row['Subtype'] in subtype_list:
            if row['Subtype'] == 'infravision':
                first_item = 'Infravision {}'.format( row['Modifier'] )
            elif row['Subtype'] == 'starting languages':
                first_item = 'Starting Languages: '
            else:
                first_item = row['Modifier']
            return_list.append( ( first_item, row['Modified'], row['Notes'] ) )
    return return_list

def get_spell_book( spell_list ):
    output_string = ''
    for spell in spell_list:
        spell['Name']
