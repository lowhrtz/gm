# -*- coding: utf-8 -*-
from WizardDefs import WizardPage
import Dice
import SystemSettings

def get_pc_gender_list():
    genList = []
    for gender in SystemSettings.gender:
        if(gender != 'NA'):
            genList.append(gender)

    return genList

def get_attribute_names():
    attrList = []
    for attribute in SystemSettings.attributes:
        attrList.append(attribute[0])

    return attrList

def dice_tuple(dice_string):
    dice_split = [d.strip() for d in dice_string.split('×')]
    dice_and_add = dice_split[0]
    dice_and_add = dice_and_add.replace('(', '').replace(')', '')
    dice_multiplier = None
    if len(dice_split) > 1:
        dice_multiplier = int(dice_split[1])
    dice_and_add_split = [daa.strip() for daa in dice_and_add.split('+')]
    base_dice = dice_and_add_split[0]
    add = None
    if len(dice_and_add_split) > 1:
        add = int(dice_and_add_split[1])
    base_dice_split = base_dice.split('d')
    dice_number = int(base_dice_split[0])
    dice_value = int(base_dice_split[1])
    return (dice_number, dice_value, add, dice_multiplier)

def dice_rating(dice_string):
    dice_number, dice_value, add, dice_multiplier = dice_tuple(dice_string)
    rating = dice_number * dice_value
    if add:
        rating += add
    if dice_multiplier:
        rating *= dice_multiplier
    return rating

def get_best_dice(dice_string_list):
    best = None
    for dice_string in dice_string_list:
        if best:
            dice_string_rating = dice_rating(dice_string)
            best_rating = dice_rating(best)
            if dice_string_rating > best_rating:
                best = dice_string
        else:
            best = dice_string

    return best

class IntroPage(WizardPage):
    page_title = "Intro"
    page_subtitle = "Intro Subtitle"
    page_id = 0
    content = "This is the wizard for creating your character."

class CharacterCreationChoicePage(WizardPage):
    page_title = "You Must Choose!"
    page_subtitle = "But choose wisely..."
    page_id = 10
    template = "infoPage"
    banner = "choose_banner.jpg"
#    banner_style = 'border: 3px inset #999999;'
    content = [
        ('text', 'How do you want to create your character?'),
        ('fillHook-vertical', 'get_choices', None, 'radiobutton', 'index'),
    ]

    def get_choices(self):
        choices = [
            'Roll Attributes First',
            'Choose Race, Class, etc. First',
        ]
        return choices

    def get_next_page_id(self, selected_radio=0):
        if selected_radio == 0:
            return RollMethodsPage.page_id
        else:
            return ChooseRacePage.page_id

class RollMethodsPage(WizardPage):
    page_title = "Roll Methods"
    page_subtitle = "Choose your roll method"
    page_id = 20
    template = "rollMethodsPage"
    attribute_list = get_attribute_names()
    content = [
        ('classic', 'Classic', '3d6'),
        ('classic+arrange', 'Classic with Arrange', '3d6'),
        ('droplow', 'Drop Lowest', '4d6'),
        ('droplow+arrange', 'Drop Lowest with Arrange', '4d6'),
        ('pool', 'Distribute from a Pool of Points', '70'),
        ('pool-forceuse', 'Distribute from a Pool of Points', '80'),
    ]

#    def get_next_page_id(self):
#        return InfoPage.page_id

class RollAttributesPage(WizardPage):
    enabled = False
    page_title = "Roll"
    page_subtitle = "Roll for you character's attributes."
    page_id = 3
    template = "infoPage"
    content = [
        ('text','Roll your attributes.'),
        ('fillHook','get_attributes',None,'spinbox',True),
        ('buTTon','Reroll','roll_action'),
        ('text','Warning: clicking "Back" will reset the scores.'),
    ]

    def get_attributes(self):
        attrList = []
        for attribute in SystemSettings.attributes:
            attrList.append(attribute[0])

        return attrList

    def roll_action(self):
        """Returns a tuple, details TBD"""
        return ('Dice.rollDice','3d6', self.get_attributes())

class InfoPage(WizardPage):
    enabled = False
    page_title = "Information"
    page_subtitle = "Please enter personal information for your character."
    page_id = 40
    template = "InfoPage"
    content = [
        ('text', 'This is the basic information that you will build your character on.'),
        ('field', 'Name', True),
        ('choose', 'Alignment', 'Settings', 'alignment'),
        ('choose', 'Race', 'DB', 'Races'),
        ('choose', 'Gender', 'Function', get_pc_gender_list),
        ('choose', 'Class', 'DB', 'Classes'),
#        ('checkbox-checked', 'This is a spellcaster', 'bool'),
    ]

#    def get_next_page_id(self, checked=True):
#        if checked:
#            return 5
#        else:
#            return 6

class ChooseRacePage(WizardPage):
    page_title = "Choose Race"
    page_subtitle = "Choose from the available races"
    page_id = 45
    layout = 'horizontal'
    template = "InfoPage"
    content = [
        ('image-method', 'RacePortrait', 'get_portrait', 'Race', 'border: 4px inset #777777;'),
    #    The _ tells GM to hide the field name from view; it's still acessable by that name minus the _
        ('listbox', 'Race_', 'method', 'get_available_races', '^$ WP{attributes} DB{races}'),
    ]

    def get_available_races(self, attribute_dict, race_dict_list):
        if attribute_dict is None:
            return [(race['Name'], race) for race in race_dict_list]

        l = []
        attribute_dict = {k.lower():int(v) for k, v in attribute_dict.items()}
        for race in race_dict_list:
            allowed = True
            for attr in get_attribute_names():
                attr = attr.capitalize()
                min_score = race['Minimum_' + attr]
                max_score = race['Maximum_' + attr]
                if not min_score <= attribute_dict[attr.lower()] <= max_score:
                    allowed = False
            if allowed:
                l.append((race['Name'], race))

        return l

    def get_portrait(self, race_dict):
        race_id = race_dict['unique_id']
        return 'portraits/Races/{filename}.jpg'.format(filename=race_id)

class ChooseClassPage(WizardPage):
    page_title = "Choose Class"
    page_subtitle = "Choose from the available classes"
    page_id = 50
    template = "infopage"
    layout = "Horizontal"
    content = [
        ('image-method', 'ClassPortrait', 'get_portrait', 'Class', 'border: 4px outset #777777;'),
        ('listbox-determinesNext', 'Class_', 'method', 'get_available_classes', '^$ WP{attributes} DB{classes} F{Race} DB{races_meta} DB{classes_meta.cols(class_id, Type, Level, Casting_Level)}', '^$ F{Class} DB{classes_meta.cols(class_id, Type, Level, Casting_Level)}'),
#        ('choose', 'Class', 'method', 'get_available_classes', '^$ WP{attributes} DB{classes} F{Race} DB{races_meta.where(class_id=Class:unique_id)}'),
    ]

    def get_available_classes(self, attribute_dict, class_dict_list, race, race_meta_dict_list, class_meta_dict_list):
        all_normal_classes = [(cl['Name'], cl) for cl in class_dict_list]
        if not race and attribute_dict is None:
            return all_normal_classes

        class_option_list = []
        for race_meta_dict in race_meta_dict_list:
            if race_meta_dict['race_id'] == race['unique_id'] and race_meta_dict['Type'] == 'class' and race_meta_dict['Subtype'] == 'permitted class options':
                class_options = race_meta_dict['Modified']
                for class_option in class_options.split(','):
                    class_option = class_option.strip()
                    if '/' in class_option:
                        multiclass_dict = {
                            'unique_id':class_option.replace('/', '_'),
                            'Name':'',
                            'Primary_Spell_List':[],
                            'classes':[],
                        }
                        name_list = []
                        for cl in class_option.split('/'):
                            class_record = self.find_class_record(cl, class_dict_list)
                            name_list.append(class_record['Name'])
                            multiclass_dict['classes'].append(class_record)
                            if class_record['Primary_Spell_List'] != 'None' and self.has_spells_at_level(1, class_meta_dict_list, class_record['unique_id']):
                                multiclass_dict['Primary_Spell_List'].append(class_record['Primary_Spell_List'])
                        multiclass_dict['Name'] = '/'.join(name_list)
                        option_tuple = (multiclass_dict['Name'], multiclass_dict)
                        class_option_list.append(option_tuple)
                    else:
                        class_record = self.find_class_record(class_option, class_dict_list)
                        option_tuple = (class_record['Name'], class_record)
                        class_option_list.append(option_tuple)

        if not class_option_list:
            class_option_list = all_normal_classes

        allowed_list = []
        if attribute_dict is not None:
            attribute_dict = {k.lower():int(v) for k, v in attribute_dict.items()}

        allowed_normal_classes = [normal_class['Name'] for normal_class in class_dict_list if self.class_allowed(normal_class, attribute_dict)]
        for class_option in class_option_list:
            class_option_allowed = True
            for class_option_item in class_option[0].split('/'):
                class_option_item = class_option_item.strip()
                if class_option_item not in allowed_normal_classes:
                    class_option_allowed = False
            if class_option_allowed:
                allowed_list.append(class_option)

        return allowed_list

    def find_class_record(self, unique_id, class_dict_list):
        for cl in class_dict_list:
            if cl['unique_id'] == unique_id:
                return cl

    def class_allowed(self, cl, attribute_dict):
        allowed = True
        minimum_scores = [i.strip() for i in cl['Minimum_Scores'].split(',')]
        for score in minimum_scores:
            score_key = score[:3].lower()
            score_value = int(score[3:].strip())
            if attribute_dict is not None and attribute_dict[score_key] < score_value:
                allowed = False
        return allowed

    def get_portrait(self, class_dict):
        class_id = class_dict['unique_id']
        return 'portraits/Classes/{filename}.jpg'.format(filename=class_id)

    def get_next_page_id(self, class_dict={}, class_meta_dict_list=[]):
        spellcaster = False
        if 'classes' in class_dict:
            for cl in class_dict['classes']:
                if cl['Primary_Spell_List'] != 'None':
                    if not spellcaster:
                        spellcaster = self.has_spells_at_level(1, class_meta_dict_list, cl['unique_id'])
        else:
            if class_dict['Primary_Spell_List'] != 'None':
                spellcaster = self.has_spells_at_level(1, class_meta_dict_list, class_dict['unique_id'])

        if spellcaster:
            return SpellsPage.page_id
        return EquipmentPage.page_id

    def has_spells_at_level(self, level, class_meta_dict_list, class_id):
        level_dict_list = [class_meta_dict for class_meta_dict in class_meta_dict_list if class_meta_dict['class_id'] == class_id and class_meta_dict['Type'] == 'xp table']
        level_dict = level_dict_list[level-1]
#        print class_id + ': ' + str(level_dict['Casting_Level'])
        if level_dict['Casting_Level'] != 0 and level_dict['Casting_Level'] != '':
            return True
        return False

class SpellsPage(WizardPage):
    page_title = 'Choose Spells'
    page_subtitle = 'Choose the spells for your character.'
    page_id = 55
    layout = 'horizontal'
    template = 'DualListPage'
    slots = ('simple','get_spell_slots','^$ F{Class} DB{classes_meta.cols(class_id, Type, Level_1_Spells, Level_1_Spells_Secondary)}')
    content = ('Spells', 'fill_list', '^$ F{Class} DB{Spells.where(Type=Class:Primary_Spell_List)}', 'Description')

    spell_types = []

    def fill_list(self, class_dict, spells_dict_list):
#        print spells_dict_list
        global multi_next_spells
        multi_next_spells = []
        if 'classes' in class_dict:
            self.spell_types = class_dict['Primary_Spell_List']
            if len(self.spell_types) > 1:
                multi_next_spells.append(self.spell_types[1])
        else:
            self.spell_types = [class_dict['Primary_Spell_List'],]
            multi_next_spells = []
        spell_list = []
        for spell_dict in spells_dict_list:
            if spell_dict['Type'] == self.spell_types[0] and spell_dict['Level'] == 1:
                spell_tuple = (spell_dict['Name'], spell_dict)
                spell_list.append(spell_tuple)
        return spell_list

    def get_spell_slots(self, class_dict, class_meta_dict_list):
        if 'classes' in class_dict:
            spell_type = class_dict['Primary_Spell_List'][0]
            spell_type = spell_type.title().replace('_', '') + ' Spells:'
        else:
            spell_type = class_dict['Name'] + ' Spells:'
        char_levels = [row for row in class_meta_dict_list if row['Type'] == 'xp table' and row['class_id'] == self.spell_types[0]]
#        print char_levels
        level_one = char_levels[0]
        return (spell_type, level_one['Level_1_Spells'])

    def get_next_page_id(self):
        if multi_next_spells:
            return SpellsPage2.page_id
        return EquipmentPage.page_id

class SpellsPage2(WizardPage):
    page_title = 'Choose Spells'
    page_subtitle = 'Choose the spells for your character.'
    page_id = 56
    layout = 'horizontal'
    template = 'DualListPage'
    slots = ('simple', 'get_spell_slots', '^$ F{Class} DB{classes_meta.cols(class_id, Type, Level_1_Spells, Level_1_Spells_Secondary)}')
    content = ('Spells2', 'fill_list', '^$ F{Class} DB{Spells.where(Type=Class:Primary_Spell_List)}', 'Description')

    def fill_list(self, class_dict, spells_dict_list):
#        print multi_next_spells
        spell_list = []
        for spell_dict in spells_dict_list:
            if spell_dict['Type'] == multi_next_spells[0] and spell_dict['Level'] == 1:
                spell_tuple = (spell_dict['Name'], spell_dict)
                spell_list.append(spell_tuple)
        return spell_list

    def get_spell_slots(self, class_dict, class_meta_dict_list):
#        print class_meta_dict_list[0]
        spell_type = multi_next_spells[0]
        spell_type = spell_type.title().replace('_', ' ') + ' Spells:'
        char_levels = [row for row in class_meta_dict_list if row['Type'] == 'xp table' and row['class_id'] == multi_next_spells[0]]
        level_one = char_levels[0]
        return (spell_type, level_one['Level_1_Spells'])

class EquipmentPage(WizardPage):
    page_title = 'Equipment'
    page_subtitle = 'Choose your equipment'
    page_id = 57
    layout = 'horizontal'
    template = 'dualListPage'
    slots = ('complex',
        'get_starting_money', '^$ F{Class}',
        'add_remove_item', '$^ F{EquipmentAvailable}',
        'add_remove_item', '$^ F{Equipment}',
        'is_complete', None)
    content = ('Equipment',
        'fill_list', '^$ F{Class} DB{Items}', '$display',
        'prefill_bought_list', '^$ F{Race} F{Class} DB{Items}')

    def fill_list(self, class_dict, items_dict_list):
        item_list = []
        for item_dict in items_dict_list:
            if item_dict['Cost'] != 'Not sold':
                item_tuple = (item_dict['Name'] + ' - ' + item_dict['Cost'], item_dict)
                item_list.append(item_tuple)

        return item_list

    def prefill_bought_list(self, race_dict, class_dict, items_dict_list):
        item_id_list = []
        if race_dict['unique_id'] == 'elf' and 'fighter' in class_dict['unique_id']:
            #percent = random.randint(1, 100)
            percent = Dice.randomInt(1, 100)
            if percent <= 5:
                item_id_list.append('armour_elfin_chain')
        if 'magic_user' in class_dict['unique_id'] or 'illusionist' in class_dict['unique_id']:
            item_id_list.append('spellbook')
        if 'cleric' in class_dict['unique_id']:
            item_id_list.append('holy_symbol_pewter')
        if 'druid' in class_dict['unique_id']:
            item_id_list.append('holy_symbol_wooden')
        if 'thief' in class_dict['unique_id'] or 'assassin' in class_dict['unique_id']:
            item_id_list.append('thieves_tools')
        item_list = []
        for item_dict in items_dict_list:
            if item_dict['unique_id'] in item_id_list:
                item_list.append((item_dict['Name'] + ' - ' + item_dict['Cost'], item_dict))

        return item_list

    def get_starting_money(self, class_dict):
        if 'classes' in class_dict:
            sm_list = []
            for cl in class_dict['classes']:
                sm_list.append(cl['Initial_Wealth_GP'])
            starting_money_string = get_best_dice(sm_list)
        else:
            starting_money_string = class_dict['Initial_Wealth_GP']
        #starting_money = roll_dice(starting_money_string)
        starting_money = Dice.rollString(starting_money_string)
        return ('Gold:', starting_money)

    def add_remove_item(self, equipment_dict):
        cost_string = equipment_dict['Cost']
        if cost_string.lower() == 'not sold':
            return False
        cost_split = cost_string.split()
        if cost_split[0].lower() == 'free':
            cost = 0
            denomination = None
        else:
            cost = ''.join(d for d in cost_split[0] if d.isdigit())
            denomination = cost_split[1]

        if denomination:
            ratio = SystemSettings.economy[denomination]
            final_cost = ratio * float(cost)
        else:
            final_cost = cost
#        print final_cost
        return (final_cost, False)

    def is_complete(self):
        return True

class InfoPage(WizardPage):
    page_title = "Misc Information"
    page_subtitle = "Choose name, alignment and gender."
    page_id = 58
    template = "InfoPage"
    content = [
        ('text', 'This is the basic information that you will build your character on.'),
        ('field', 'Name', True),
        ('choose', 'Alignment', 'method', 'get_availale_alignments', '^$ F{Class}'),
        ('choose', 'Gender', 'Function', get_pc_gender_list),
    ]

    def get_availale_alignments(self, class_dict):
        alignment_options = []
        if 'classes' in class_dict:
            for cl in class_dict['classes']:
                alignment_options.append(cl['Alignment'])
        else:
            alignment_options.append(class_dict['Alignment'])

        alignment_list = list(SystemSettings.alignment)
        for align_option in alignment_options:
            if align_option == 'Any Good':
                for align in SystemSettings.alignment:
                    if align.endswith('Neutral') or align.endswith('Evil'):
                        alignment_list.remove(align)

            elif align_option == 'Any Evil':
                for align in SystemSettings.alignment:
                    if align.endswith('Neutral') or align.endswith('Good'):
                        alignment_list.remove(align)

            elif align_option == 'Any Neutral or Evil':
                for align in SystemSettings.alignment:
                    if align.endswith('Good'):
                        alignment_list.remove(align)

            elif align_option == 'Neutral only':
                alignment_list = ['True Neutral',]

            elif align_option.lower().endswith('only'):
                alignment_list = [align_option[:-5],]

        return alignment_list

class ReviewPage(WizardPage):
    page_title = "Review"
    page_subtitle = "Review Subtitle"
    page_id = 60
    layout = 'horizontal'
    template = 'InfoPage'
    content = [
        ('text', '''\
<b>Name:</b> F{Name}<br />
<b>Gender:</b> F{Gender}<br />
<b>Alignment:</b> F{Alignment}<br />
<b>Race:</b> F{Race}<br />
<b>Class:</b> F{Class}<br />
<b>Primary Spells:</b> F{SpellsList.Name}<br />
<b>Secondary Spells:</b> F{Spells2List.Name}<br />''', True),
        ('text', 'WP{attributes.roll_attributes(F{Race}, F{Class})}', True),
    ]

    def adjust_attributes(self, race_dict):
        pass

    def roll_attributes(self, race_dict, class_dict):
        attr_dict = {}
        min_dict = {}
        if 'classes' in class_dict:
            for cl in class_dict['classes']:
                min_scores_string = cl['Minimum_Scores']
                min_scores_list = [score.strip() for score in min_scores_string.split(',')]
                for min_score in min_scores_list:
                    min_score_split = min_score.split()
                    attr = min_score_split[0]
                    min_score = int(min_score_split[1])
                    if attr not in min_dict:
                        min_dict[attr] = min_score
                    else:
                        min_dict[attr] = max(min_score, min_dict[attr])
        else:
            min_scores_string = class_dict['Minimum_Scores']
            min_scores_list = [score.strip() for score in min_scores_string.split(',')]
            for min_score in min_scores_list:
                min_score_split = min_score.split()
                attr = min_score_split[0]
                min_score = int(min_score_split[1])
                min_dict[attr] = min_score

        if len(min_dict) < 6:
            for attr in get_attribute_names():
                if attr.title() not in min_dict:
                    min_dict[attr.title()] = 3

        for attr in min_dict:
            minimum = max(min_dict[attr], race_dict['Minimum_' + attr])
            maximum = race_dict['Maximum_' + attr]
            score = Dice.randomInt(minimum, maximum)
            attr_dict[attr.upper()] = str(score)

        return attr_dict
