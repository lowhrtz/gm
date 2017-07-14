# -*- coding: utf-8 -*-
from string import Template
from WizardDefs import WizardPage
import base64
import DbQuery
import Dice
import os
import SystemSettings

def get_pc_gender_list():
    genList = []
    for gender in SystemSettings.gender:
        if( gender != 'NA' ):
            genList.append( gender )

    return genList

def get_attribute_names():
    attrList = []
    for attribute in SystemSettings.attributes:
        attrList.append( attribute[0] )

    return attrList

def dice_tuple( dice_string ):
    dice_split = [ d.strip() for d in dice_string.split('×') ]
    dice_and_add = dice_split[0]
    dice_and_add = dice_and_add.replace( '(', '' ).replace( ')', '' )
    dice_multiplier = None
    if len( dice_split ) > 1:
        dice_multiplier = int( dice_split[1] )
    dice_and_add_split = [ daa.strip() for daa in dice_and_add.split('+') ]
    base_dice = dice_and_add_split[0]
    add = None
    if len( dice_and_add_split ) > 1:
        add = int( dice_and_add_split[1] )
    base_dice_split = base_dice.split('d')
    dice_number = int( base_dice_split[0] )
    dice_value = int( base_dice_split[1] )
    return ( dice_number, dice_value, add, dice_multiplier )

def dice_rating( dice_string ):
    dice_number, dice_value, add, dice_multiplier = dice_tuple(dice_string)
    rating = dice_number * dice_value
    if add:
        rating += add
    if dice_multiplier:
        rating *= dice_multiplier
    return rating

def get_best_dice( dice_string_list ):
    best = None
    for dice_string in dice_string_list:
        if best:
            dice_string_rating = dice_rating( dice_string )
            best_rating = dice_rating( best )
            if dice_string_rating > best_rating:
                best = dice_string
        else:
            best = dice_string

    return best

class IntroPage( WizardPage ):
    page_title = "Intro"
    page_subtitle = "Intro Subtitle"
    page_id = 0
    content = "This is the wizard for creating your character."

class CharacterCreationChoicePage( WizardPage ):
    page_title = "You Must Choose!"
    page_subtitle = "But choose wisely..."
    page_id = 10
    template = "infoPage"
    banner = "choose_banner.jpg"
    content = [
        ( 'text', 'How do you want to create your character?' ),
        ( 'fillHook-CreationChoice-vertical', 'get_choices', None, 'radiobutton', 'index' ),
    ]

    def get_choices( self ):
        choices = [
            'Roll Attributes First',
            'Choose Race, Class, etc. First',
        ]
        return choices

    def get_next_page_id( self ):
        selected_radio = self.fields['CreationChoice']
        if selected_radio == 0:
            return RollMethodsPage.page_id
        else:
            return ChooseRacePage.page_id

class RollMethodsPage( WizardPage ):
    page_title = "Roll Methods"
    page_subtitle = "Choose your roll method"
    page_id = 20
    template = "rollMethodsPage"
    attribute_list = get_attribute_names()
    content = [
        ( 'classic', 'Classic', '3d6' ),
        ( 'classic+arrange', 'Classic with Arrange', '3d6' ),
        ( 'droplow', 'Drop Lowest', '4d6' ),
        ( 'droplow+arrange', 'Drop Lowest with Arrange', '4d6' ),
        ( 'pool', 'Distribute from a Pool of Points', '70' ),
        ( 'pool-forceuse', 'Distribute from a Pool of Points', '80' ),
    ]

class ChooseRacePage( WizardPage ):
    page_title = "Choose Race"
    page_subtitle = "Choose from the available races"
    page_id = 50
    layout = 'horizontal'
    template = "InfoPage"
    content = [
        ( 'image-method', 'RacePortrait', 'get_portrait', 'Race', 'border: 4px inset #777777;' ),
        #The _ tells GM to hide the field name from view; it's still acessable by that name minus the _
        ( 'listbox', 'Race_', 'method', 'get_available_races', None ),
    ]

    def get_available_races( self ):
        race_dict_list = DbQuery.getTable( 'Races' )

        #print 'STR: ' + self.fields['STR']
        if self.fields['STR']:
            attribute_dict = { attr : int( self.fields[attr] ) for attr in get_attribute_names() }
        else:
            attribute_dict = None

        if attribute_dict is None:
            return [ ( race['Name'], race ) for race in race_dict_list ]

        l = []
        for race in race_dict_list:
            allowed = True
            for attr in get_attribute_names():
                attr_cap = attr.capitalize()
                min_score = race[ 'Minimum_' + attr_cap ]
                max_score = race[ 'Maximum_' + attr_cap ]
                if not min_score <= attribute_dict[attr] <= max_score:
                    allowed = False
            if allowed:
                l.append( ( race['Name'], race ) )

        return l

    def get_portrait( self, race_dict ):
        race_id = race_dict['unique_id']
        return 'portraits/Races/{filename}.jpg'.format( filename=race_id )

class ChooseClassPage( WizardPage ):
    page_title = "Choose Class"
    page_subtitle = "Choose from the available classes"
    page_id = 60
    template = "infopage"
    layout = "Horizontal"
    content = [
        ( 'image-method', 'ClassPortrait', 'get_portrait', 'Class', 'border: 4px outset #777777;' ),
        ( 'listbox', 'Class_', 'method', 'get_available_classes', None ),
    ]

    def get_available_classes( self ):
        if self.fields['STR']:
            attribute_dict = { attr : self.fields[attr] for attr in get_attribute_names() }
        else:
            attribute_dict = None
        class_dict_list = DbQuery.getTable( 'Classes' )
        race = self.fields['Race']
        all_normal_classes = [ ( cl['Name'], cl ) for cl in class_dict_list ]
        if not race and attribute_dict is None:
            return all_normal_classes

        class_option_list = []
        for race_meta_dict in race['Races_meta']:
            if race_meta_dict['Type'] == 'class' and race_meta_dict['Subtype'] == 'permitted class options':
                class_options = race_meta_dict['Modified']
                for class_option in class_options.split( ',' ):
                    class_option = class_option.strip()
                    if '/' in class_option:
                        multiclass_dict = {
                            'unique_id' : class_option.replace( '/', '_' ),
                            'Name' : '',
                            'Primary_Spell_List' : [],
                            'classes' : [],
                        }
                        name_list = []
                        for cl in class_option.split( '/' ):
                            class_record = self.find_class_record( cl, class_dict_list )
                            name_list.append( class_record['Name'] )
                            multiclass_dict['classes'].append( class_record )
                            if class_record['Primary_Spell_List'] != 'None' and SystemSettings.has_spells_at_level( 1, class_record ):
                                multiclass_dict['Primary_Spell_List'].append( class_record['Primary_Spell_List'] )
                        multiclass_dict['Name'] = '/'.join( name_list )
                        option_tuple = ( multiclass_dict['Name'], multiclass_dict )
                        class_option_list.append( option_tuple )
                    else:
                        class_record = self.find_class_record( class_option, class_dict_list )
                        option_tuple = ( class_record['Name'], class_record )
                        class_option_list.append( option_tuple )

        if not class_option_list:
            class_option_list = all_normal_classes

        allowed_list = []
        if attribute_dict is not None:
            attribute_dict = {k.lower():int(v) for k, v in attribute_dict.items()}

        allowed_normal_classes = [ normal_class['Name'] for normal_class in class_dict_list if self.class_allowed( normal_class, attribute_dict ) ]
        for class_option in class_option_list:
            class_option_allowed = True
            for class_option_item in class_option[0].split('/'):
                class_option_item = class_option_item.strip()
                if class_option_item not in allowed_normal_classes:
                    class_option_allowed = False
            if class_option_allowed:
                allowed_list.append( class_option )

        return allowed_list

    def find_class_record( self, unique_id, class_dict_list ):
        for cl in class_dict_list:
            if cl['unique_id'] == unique_id:
                return cl

    def class_allowed( self, cl, attribute_dict ):
        allowed = True
        minimum_scores = [ i.strip() for i in cl['Minimum_Scores'].split( ',' ) ]
        for score in minimum_scores:
            score_key = score[:3].lower()
            score_value = int( score[3:].strip() )
            if attribute_dict is not None and attribute_dict[score_key] < score_value:
                allowed = False
        return allowed

    def get_portrait( self, class_dict ):
        class_id = class_dict['unique_id']
        return 'portraits/Classes/{filename}.jpg'.format(filename=class_id)

    def get_next_page_id( self ):
        class_dict = self.fields['Class']
        if not class_dict:
            return -2
        wizard = False
        self.wizard_type = None
        spellcaster = False
        if 'classes' in class_dict:
            for cl in class_dict['classes']:
                if cl['Primary_Spell_List'] != 'None':
                    if cl['Category'] == 'wizard':
                        wizard = True
                        self.wizard_type = cl['unique_id']
                    if not spellcaster:
                        spellcaster = SystemSettings.has_spells_at_level( 1, cl )
        else:
            if class_dict['Primary_Spell_List'] != 'None':
                if class_dict['Category'] == 'wizard':
                    wizard = True
                    self.wizard_type = class_dict['unique_id']
                spellcaster = SystemSettings.has_spells_at_level( 1, class_dict )

        if wizard and spellcaster:
            return SpellbookPage.page_id
        if spellcaster:
            return SpellsPage.page_id
        return ProficiencyPage.page_id

def prefill_chosen_spells( spell_type, spells_dict_list ):
    prechosen_ids = []
    prechosen_spells = []
    avail_list = [spell_dict for spell_dict in spells_dict_list if spell_dict['Type'] == spell_type and spell_dict['Level'] == 1]
    if spell_type == 'magic_user':
        prechosen_ids.append('read_magic')
        for spell in avail_list:
            if spell['spell_id'] == 'read_magic':
                avail_list.remove(spell)

    if spell_type == 'magic_user' or spell_type == 'illusionist':
        first_random = Dice.randomInt(0, len(avail_list) - 1)
        first_random_spell = avail_list.pop(first_random)
        prechosen_ids.append(first_random_spell['spell_id'])

        second_random = Dice.randomInt(0, len(avail_list) - 1)
        second_random_spell = avail_list.pop(second_random)
        prechosen_ids.append(first_random_spell['spell_id'])
        prechosen_ids.append(second_random_spell['spell_id'])

    for spell_dict in spells_dict_list:
        if spell_dict['spell_id'] in prechosen_ids:
            prechosen_spells.append((spell_dict['Name'], spell_dict, True))
    return prechosen_spells

class SpellbookPage( WizardPage ):
    page_title = 'Choose Spells'
    page_subtitle = 'Choose the spell(s) in your spell book.'
    page_id = 65
    layout = 'horizontal'
    template = 'DualListPage'
    slots = ( 'simple','get_spell_slots', '' )
    content = ( 'Spellbook',
                'fill_list', '', 'Description',
                'prefill_chosen_spells', '' )

    def get_spell_slots( self ):
        if self.wizard_type == 'magic_user':
            return ( 'Magic User Spells: ', 1 )
        else:
            return ( '{} Spells: '.format( self.wizard_type.title().replace( '_', ' ' ) ), 2 )

    def fill_list( self ):
        spells_dict_list = DbQuery.getTable( 'Spells' )
        self.wizard_type = self.pages['ChooseClassPage'].wizard_type
        spell_list = []
        for spell_dict in spells_dict_list:
            if spell_dict['Type'] == self.wizard_type and spell_dict['Level'] == 1:
                spell_tuple = ( spell_dict['Name'], spell_dict )
                spell_list.append( spell_tuple )
        return spell_list

    def prefill_chosen_spells( self ):
        spells_dict_list = DbQuery.getTable( 'Spells' )
        return prefill_chosen_spells( self.wizard_type, spells_dict_list )

def get_spell_slots( class_dict, index=0 ):
    if 'classes' in class_dict:
        spell_type = class_dict['Primary_Spell_List'][index]
        spell_type_label = spell_type.title().replace( '_', ' ' ) + ' Spells:'
        if class_dict['classes'][index]['Classes_meta'][0]['class_id'] == spell_type:
            class_meta_dict_list =  class_dict['classes'][index]['Classes_meta']
        else:
            class_meta_dict_list =  class_dict['classes'][index + 1]['Classes_meta']
    else:
        spell_type = class_dict['Primary_Spell_List']
        spell_type_label = class_dict['Name'] + ' Spells:'
        class_meta_dict_list = class_dict['Classes_meta']

    char_levels = [ row for row in class_meta_dict_list if row['Type'] == 'xp table' and row['class_id'] == spell_type ]
    level_one = char_levels[0]
    return ( spell_type_label, level_one['Level_1_Spells'] )

class SpellsPage( WizardPage ):
    page_title = 'Choose Spells'
    page_subtitle = 'Choose the typical daily spell(s) for your character.'
    page_id = 70
    layout = 'horizontal'
    template = 'DualListPage'
    slots = ( 'simple', 'get_spell_slots', '' )
    content = ('Spells',
               'fill_list', '', 'Description' )

    multi_next_spell_type = ''

    def fill_list( self ):
        class_dict = self.fields['Class']
        spells_dict_list = DbQuery.getTable( 'Spells' )
        if 'classes' in class_dict:
            self.spell_types = class_dict['Primary_Spell_List']
            if len(self.spell_types) > 1:
                self.multi_next_spell_type = self.spell_types[1]
        else:
            self.spell_types = [ class_dict['Primary_Spell_List'], ]
        spell_list = []
        if self.spell_types[0] == self.pages['ChooseClassPage'].wizard_type:
            for spell_dict in self.fields['SpellbookList']:
                spell_tuple = ( spell_dict['Name'], spell_dict )
                spell_list.append( spell_tuple )
        else:
            for spell_dict in spells_dict_list:
                if spell_dict['Type'] == self.spell_types[0] and spell_dict['Level'] == 1:
                    spell_tuple = ( spell_dict['Name'], spell_dict )
                    spell_list.append( spell_tuple )

        return spell_list

    def get_spell_slots( self ):
        class_dict = self.fields['Class']
        return get_spell_slots( class_dict )

    def get_next_page_id( self ):
        if self.multi_next_spell_type:
            return SpellsPage2.page_id
        return ProficiencyPage.page_id


class SpellsPage2( WizardPage ):
    page_title = 'Choose Spells'
    page_subtitle = 'Choose the typical daily spell(s) for your character.'
    page_id = 80
    layout = 'horizontal'
    template = 'DualListPage'
    slots = ('simple', 'get_spell_slots', '')
    content = ( 'Spells2',
               'fill_list', '', 'Description' )

    def fill_list( self ):
        spells_dict_list = DbQuery.getTable( 'Spells' )
        spell_list = []

        if self.pages['SpellsPage'].multi_next_spell_type == self.pages['ChooseClassPage'].wizard_type:
            for spell_dict in self.fields['SpellbookList']:
                spell_tuple = (spell_dict['Name'], spell_dict)
                spell_list.append( spell_tuple )
        else:
            for spell_dict in spells_dict_list:
                if spell_dict['Type'] == self.pages['SpellsPage'].multi_next_spell_type and spell_dict['Level'] == 1:
                    spell_tuple = (spell_dict['Name'], spell_dict)
                    spell_list.append( spell_tuple )
        return spell_list

    def get_spell_slots( self ):
        class_dict = self.fields['Class']
        return get_spell_slots( class_dict, 1 )

class ProficiencyPage( WizardPage ):
    page_title = 'Weapon Proficiency'
    page_subtitle = 'Choose your proficiencies'
    page_id = 85
    layout = 'horizontal'
    template = 'dualListPage'
    slots = ( 'simple', 'get_slots', '' )
    content = ( 'Proficiency', 'fill_proficiencies', '' )

    def get_slots( self ):
        class_dict = self.fields['Class']
        race_dict = self.fields['Race']
        if 'classes' in class_dict:
            cl_slots = []
            for cl in class_dict['classes']:
                cl_slots.append( cl['Initial_Weapon_Proficiencies'] )
            if race_dict['unique_id'] in SystemSettings.restrictive_races:
                slots = min( cl_slots )
            else:
                slots = max( cl_slots )
        else:
            slots = class_dict['Initial_Weapon_Proficiencies']
        return ( 'Proficiency Slots:', slots )

    def race_wp( self, wp_list, race_id, item_dict_list ):
        blunt_list = [ blunt['Name'].lower() for blunt in item_dict_list if 'blunt' in blunt['Damage_Type'].split( ',' ) ]
        race_wp = []
        wp_expand = []
        for wp in wp_list:
            wp_expand.append( [ w.strip().lower() for w in wp.split( ',' ) ] )
        if race_id in SystemSettings.restrictive_races:
            bucket = wp_expand[0]
            if 'blunt' in bucket:
                bucket.remove( 'blunt' )
                bucket.extend( blunt_list )
            for i in range( 1, len( wp_expand ) ):
                if 'blunt' in wp_expand[i]:
                    wp_expand[i].remove( 'blunt' )
                    wp_expand[i].extend( blunt_list )
                if 'any' in bucket:
                    bucket = wp_expand[i]
                elif 'any' in wp_expand[i]:
                    bucket = bucket
                else:
                    bucket = [ w for w in bucket if w in wp_expand[i] ]

        else:
            wp_flat = sum( wp_expand, [] )
            bucket = []
            for wp in wp_flat:
                if wp not in bucket:
                    bucket.append( wp )
        if 'any' in bucket:
            bucket = [ 'any', ]
        return bucket

    def fill_proficiencies( self ):
        class_dict = self.fields['Class']
        race_dict = self.fields['Race']
        item_dict_list = [ i for i in DbQuery.getTable( 'Items' ) if i['Is_Proficiency'] == 'yes' ]
        if 'classes' in class_dict:
            wp_list = [ cl['Weapons_Permitted' ] for cl in class_dict[ 'classes' ] ]
            weapons_permitted = self.race_wp( wp_list, race_dict['unique_id'], item_dict_list )
        else:
            weapons_permitted = [ weapon.strip().lower() for weapon in class_dict['Weapons_Permitted'].split( ',' ) ]
        item_list = []
        for item_dict in item_dict_list:
            item_tuple = ( item_dict['Name'], item_dict )
            damage_type_list = [ damage_type.strip().lower() for damage_type in item_dict['Damage_Type'].split( ',' ) ]
            if 'any' in weapons_permitted:
                item_list.append( item_tuple )
            elif any( weapon in item_dict['Name'].lower() for weapon in weapons_permitted ):
                item_list.append( item_tuple )
            elif [ i for i in weapons_permitted if i in damage_type_list ]:
                item_list.append( item_tuple )
            elif 'single-handed swords (except bastard swords)' in weapons_permitted:
                if item_dict['unique_id'].startswith( 'sword' ) and \
                'both-hand' not in damage_type_list and 'two-hand' not in damage_type_list:
                    item_list.append( item_tuple )
        return item_list

class EquipmentPage( WizardPage ):
    page_title = 'Equipment'
    page_subtitle = 'Choose your equipment'
    page_id = 90
    layout = 'horizontal'
    template = 'dualListPage'
    slots = ( 'complex',
        'get_starting_money', '',
        'add_item', '',
        'remove_item', '',
        'is_complete', None)
    content = ('Equipment',
        'fill_list', '', '$display',
        'prefill_bought_list', '')

    def fill_list( self ):
        class_dict = self.fields['Class']
        items_dict_list = DbQuery.getTable( 'Items' )
        item_list = []
        for item_dict in items_dict_list:
            if item_dict['Cost'].lower() != 'not sold' and not item_dict['Cost'].lower().startswith( 'proficiency' ):
                item_tuple = ( item_dict['Name'] + ' - ' + item_dict['Cost'], item_dict )
                item_list.append( item_tuple )

        return item_list

    def prefill_bought_list( self ):
        race_dict = self.fields['Race']
        class_dict = self.fields['Class']
        items_dict_list = DbQuery.getTable( 'Items' )
        item_id_list = []
        if race_dict['unique_id'] == 'elf' and 'fighter' in class_dict['unique_id']:
            percent = Dice.randomInt(1, 100)
            if percent <= 5:
                item_id_list.append( 'armour_elfin_chain' )
        if 'magic_user' in class_dict['unique_id'] or 'illusionist' in class_dict['unique_id']:
            item_id_list.append( 'spellbook' )
        if 'cleric' in class_dict['unique_id']:
            item_id_list.append( 'holy_symbol_pewter' )
        if 'druid' in class_dict['unique_id']:
            item_id_list.append( 'holy_symbol_wooden' )
        if 'thief' in class_dict['unique_id'] or 'assassin' in class_dict['unique_id']:
            item_id_list.append( 'thieves_tools' )
        item_list = []
        for item_dict in items_dict_list:
            if item_dict['unique_id'] in item_id_list:
                item_list.append( ( item_dict['Name'] + ' - ' + item_dict['Cost'], item_dict ) )

        return item_list

    def get_starting_money( self ):
        class_dict = self.fields['Class']
        if 'classes' in class_dict:
            sm_list = []
            for cl in class_dict['classes']:
                sm_list.append( cl['Initial_Wealth_GP'] )
            starting_money_string = get_best_dice( sm_list )
        else:
            starting_money_string = class_dict['Initial_Wealth_GP']
        starting_money = Dice.rollString( starting_money_string )
        return ( 'Gold:', starting_money )

    def add_item( self ):
        equipment_dict = self.fields['EquipmentAvailable']
        return self.add_remove_item( equipment_dict )

    def remove_item( self ):
        equipment_dict = self.fields['Equipment']
        return self.add_remove_item( equipment_dict )

    def add_remove_item( self, equipment_dict ):
        cost_string = equipment_dict['Cost']
        if cost_string.lower() == 'not sold':
            return False
        cost_split = cost_string.split()
        if cost_split[0].lower() == 'free':
            cost = 0
            denomination = None
        else:
            cost = ''.join(d for d in cost_split[0] if d.isdigit())
            try:
                denomination = cost_split[1]
            except KeyError:
                denomination = None

        if denomination:
            try:
                ratio = SystemSettings.economy[denomination]
                final_cost = ratio * float( cost )
            except KeyError:
                final_cost = cost
        else:
            final_cost = cost

        return ( final_cost, False )

    def is_complete( self ):
        return True

class InfoPage( WizardPage ):
    page_title = "Misc Information"
    page_subtitle = "Choose name, alignment and gender."
    page_id = 100
    template = "InfoPage"
    content = [
        ( 'text', 'This is the basic information that you will build your character on.' ),
        ( 'field', 'Name', True ),
        ( 'choose', 'Alignment', 'method', 'get_availale_alignments', '' ),
        ( 'choose', 'Gender', 'Function', get_pc_gender_list ),
    ]

    def get_availale_alignments( self ):
        class_dict = self.fields['Class']
        alignment_options = []
        if 'classes' in class_dict:
            for cl in class_dict['classes']:
                alignment_options.append( cl['Alignment'] )
        else:
            alignment_options.append( class_dict['Alignment'] )

        alignment_list = list( SystemSettings.alignment )
        for align_option in alignment_options:
            if align_option == 'Any Good':
                for align in SystemSettings.alignment:
                    if align.endswith( 'Neutral' ) or align.endswith( 'Evil' ):
                        alignment_list.remove( align )

            elif align_option == 'Any Evil':
                for align in SystemSettings.alignment:
                    if align.endswith( 'Neutral' ) or align.endswith( 'Good' ):
                        alignment_list.remove( align )

            elif align_option == 'Any Neutral or Evil':
                for align in SystemSettings.alignment:
                    if align.endswith( 'Good' ):
                        alignment_list.remove( align )

            elif align_option == 'Neutral only':
                alignment_list = [ 'True Neutral', ]

            elif align_option.lower().endswith( 'only' ):
                alignment_list = [ align_option[:-5], ]

        return alignment_list

class ReviewPage( WizardPage ):
    page_title = "Review"
    page_subtitle = "Review Subtitle"
    page_id = 110
    layout = 'horizontal'
    template = 'InfoPage'

    attr_cache = None
    hp_cache = None
    height_weight_cache = None
    age_cache = None

    def __init__(self):
        self.content = [
            ('text', '''\
<b>Name:</b> F{Name}<br />
<b>Gender:</b> F{Gender}<br />
<b>Align:</b> F{Alignment}<br />
<b>Race:</b> F{Race}<br />
<b>Class:</b> F{Class}<br />
<br />
MD{roll_attributes(WP{attributes}, F{Race}, F{Class})}''', True),
            ('listbox-halfwidth', 'All Spells', 'method', 'fill_spells1', ''),
            ('listbox-halfwidth', ' ', 'method', 'fill_spells2', ''),
            ('listbox-halfwidth', 'Proficiencies', 'method', 'fill_proficiencies', ''),
            ('listbox-halfwidth', 'Equip', 'method', 'fill_equipment', ''),
            ('button', 'Create PDF', 'printpdf', self.get_pdf_markup, ''),
        ]

    def roll_hp( self, level, attr_dict, class_dict ):
        hp = 0
        con_score = attr_dict['CON']
        con_bonus = SystemSettings.get_attribute_bonuses( 'CON', con_score )[0]
        con_bonus_list = con_bonus.replace( ' for Warriors)', '' ).split(' (')
        if len(con_bonus_list) == 1:
            con_bonus_list.append( con_bonus_list[0] )
        if 'classes' in class_dict:
            for cl in class_dict['classes']:
                hp_temp = 0
                if cl['Category'].lower() == 'warrior':
                    con_bonus = con_bonus_list[1]
                else:
                    con_bonus = con_bonus_list[0]
                hit_dice_string = 'd{}'.format( cl['Hit_Die_Type'] )
                for i in range( 0, int(level) ):
                    hp_temp += Dice.rollString( hit_dice_string )
                    hp_temp += int( con_bonus )
                hp += hp_temp / len( class_dict['classes'] )
        else:
            if class_dict['Category'].lower() == 'warrior':
                con_bonus = con_bonus_list[1]
            else:
                con_bonus = con_bonus_list[0]
            hit_dice_string = 'd{}'.format(class_dict['Hit_Die_Type'])
            for i in range(0, int(level)):
                hp += Dice.rollString(hit_dice_string)
                hp += int(con_bonus)

        return hp

    def roll_age( self ):
        race_dict = self.fields['Race']
        class_dict = self.fields['Class']

        starting_ages = [ row for row in race_dict['Races_meta'] if row['Type'] == 'class' and row['Subtype'] == 'starting age' ]

        class_groups = {'Cleric': ['cleric', 'druid'],
                        'Fighter': ['fighter', 'ranger', 'paladin'],
                        'Magic User': ['magic_user', 'illusionist'],
                        'Thief': ['thief', 'assassin'],
                        'Druid': ['druid'],
                        }

        dice_string = ''
        if 'classes' in class_dict:
            bucket = []
            for cl in class_dict['classes']:
                for row in starting_ages:
                    if cl['unique_id'] in class_groups[row['Modified']]:
                        bucket.append( row )
            rating = 0
            best_dice = ''
            for row in bucket:
                new_rating = eval( row['Modifier'].replace( 'd', '*' ) )
                if new_rating > rating:
                    rating = new_rating
                    best_dice = row['Modifier']
            dice_string = best_dice
        else:
            for row in starting_ages:
                if class_dict['unique_id'] in class_groups[ row['Modified'] ]:
                    dice_string = row['Modifier']


        dice_string_list = dice_string.split( '+' )
        dice_string = dice_string_list[1].strip() + '+' + dice_string_list[0].strip()
        return Dice.rollString( dice_string )

    def roll_height_weight( self ):
        race_dict = self.fields['Race']
        gender = self.fields['Gender']

        height_table = [ row for row in race_dict['Races_meta'] if row['Type'] == 'height table' and row['Subtype'].lower() == gender.lower() ]
        weight_table = [ row for row in race_dict['Races_meta'] if row['Type'] == 'weight table' and row['Subtype'].lower() == gender.lower() ]

        height_roll = Dice.randomInt( 1, 100 )
        weight_roll = Dice.randomInt( 1, 100 )

        def lookup( roll, table ):
            for row in table:
                d = row['Modifier']
                result = row['Modified']
                bounds = [ int(b) for b in d.split('-') ]
                if roll >= bounds[0] and roll <= bounds[1]:
                    return result

        height_result = lookup( height_roll, height_table )
        weight_result = lookup( weight_roll, weight_table )

        height_result_list = height_result.split( '+' )
        weight_result_list = weight_result.split( '+' )

        height_base = height_result_list[0].split()
        height_base_in = int( height_base[0] ) * 12 + int( height_base[2] )
        height_mod = height_result_list[1].replace( ' in', '' )
        weight_base = weight_result_list[0].replace( ' lbs', '' )
        weight_mod = weight_result_list[1].replace( ' lbs', '' )

        height = height_base_in + Dice.rollString( height_mod )
        weight = int(weight_base) + Dice.rollString(weight_mod)

        last_height_roll = Dice.rollString( 'd6' )
        last_weight_roll = Dice.rollString( 'd6' )

        while last_height_roll == 1 or last_height_roll == 6:
            height_sign = 1
            if last_height_roll == 1:
                height_sign = -1
            height = height + height_sign * Dice.rollString( '1d4' )
            last_height_roll = Dice.rollString( 'd6' )

        while last_weight_roll == 1 or last_weight_roll == 6:
            weight_sign = 1
            if last_weight_roll == 1:
                weight_sign = -1
            weight = weight + weight_sign * Dice.rollString( '1d20' )
            last_weight_roll = Dice.rollString( 'd6' )

        height_tuple = ( height / 12, height % 12 )

        return ( height_tuple, weight )


    def adjust_attributes( self, attr_dict, race_dict ):
        for meta_row in race_dict['Races_meta']:
            if meta_row['Type'] == 'ability' and meta_row['Subtype'] == 'attribute':
                attr_to_modify = meta_row['Modified'].upper()[:3]
                modifier = meta_row['Modifier']
                attr_orig_score = attr_dict[attr_to_modify]
                new_score = eval( attr_orig_score + modifier )
                attr_dict[attr_to_modify] = str( new_score )
        return attr_dict

    def roll_attributes( self, wiz_attr_dict, race_dict, class_dict ):
        is_warrior = False
        if 'classes' in class_dict:
            for cl in class_dict['classes']:
                if cl['Category'].lower() == 'warrior':
                    is_warrior = True
        else:
            if class_dict['Category'].lower() == 'warrior':
                is_warrior = True

        attr_dict = {}
        min_dict = {}
        if not wiz_attr_dict:
            if 'classes' in class_dict:
                for cl in class_dict['classes']:
                    min_scores_string = cl['Minimum_Scores']
                    min_scores_list = [ score.strip() for score in min_scores_string.split( ',' ) ]
                    for min_score in min_scores_list:
                        min_score_split = min_score.split()
                        attr = min_score_split[0]
                        min_score = int( min_score_split[1] )
                        if attr not in min_dict:
                            min_dict[attr] = min_score
                        else:
                            min_dict[attr] = max( min_score, min_dict[attr] )
            else:
                min_scores_string = class_dict['Minimum_Scores']
                min_scores_list = [ score.strip() for score in min_scores_string.split( ',' ) ]
                for min_score in min_scores_list:
                    min_score_split = min_score.split()
                    attr = min_score_split[0]
                    min_score = int( min_score_split[1] )
                    min_dict[attr] = min_score

            if len( min_dict ) < 6:
                for attr in get_attribute_names():
                    if attr.title() not in min_dict:
                        min_dict[ attr.title() ] = 3

            for attr in min_dict:
                minimum = max( min_dict[attr], race_dict['Minimum_' + attr] )
                maximum = race_dict[ 'Maximum_' + attr ]
                score = Dice.randomInt( minimum, maximum )
                attr_dict[ attr.upper() ] = str( score )
        else:
            attr_dict = self.adjust_attributes( wiz_attr_dict, race_dict )

        for attr in attr_dict:
            if attr.lower() == 'str' and attr_dict[attr] == '18' and is_warrior:
                score = 18
                exceptional_strength = Dice.randomInt( 1, 99 ) / float( 100 )
                attr_dict[attr] = '{0:.2f}'.format( score + exceptional_strength )

        self.attr_cache = attr_dict

        # Now that we have guaranteed attributes lets roll for hp and cache the results
        self.hp_cache = self.roll_hp( 1, attr_dict, class_dict )

        # Might as well roll age, height and weight too
        self.height_weight_cache = self.roll_height_weight()
        self.age_cache = self.roll_age()

        return '''\
<b>Str:</b> {STR}<br />
<b>Int:</b> {INT}<br />
<b>Wis:</b> {WIS}<br />
<b>Dex:</b> {DEX}<br />
<b>Con:</b> {CON}<br />
<b>Cha:</b> {CHA}
    '''.format(**attr_dict)

    def fill_items( self, items ):
        if len( items ) > 0:
            return [ ( item['Name'], item ) for item in items ]
        else:
            return []

    def fill_spells1( self ):
        spells_dict_list = self.fields['SpellsList']
        return self.fill_items( spells_dict_list )

    def fill_spells2( self ):
        spells_dict_list = self.fields['Spells2List']
        return self.fill_items( spells_dict_list )

    def fill_proficiencies( self ):
        proficiencies_dict_list = self.fields['ProficiencyList']
        return self.fill_items( proficiencies_dict_list )

    def fill_equipment( self ):
        equip_dict_list = self.fields['EquipmentList']
        return self.fill_items( equip_dict_list )

    def get_attr_bonuses_string( self, attr_key, score ):
        bonuses_dict = {}
        bonuses_dict['STR'] = 'To Hit: {}     Damage: {}     Encumbrance: {}     Minor Test: {}     Major Test: {}%'
        bonuses_dict['INT'] = 'Additional Languages: {}'
        bonuses_dict['WIS'] = 'Mental Save: {}     Bonus Cleric Spells: {}     Cleric Spell Failure: {}%'
        bonuses_dict['DEX'] = 'Surprise: {}     Missile To Hit: {}     AC Adjustment: {}'
        bonuses_dict['CON'] = 'HP Bonus: {}   Resurrect/Raise Dead: {}%   System Shock: {}%'
        bonuses_dict['CHA'] = 'Max Henchman: {}     Loyalty: {}     Reaction: {}'

        return bonuses_dict[attr_key].format( *SystemSettings.get_attribute_bonuses( attr_key, score ) )

    def get_pdf_markup( self ):
        level = 1
        class_dict = self.fields['Class']
        class_name = class_dict['Name']
        class_font_size = '14px'
        class_padding = '0px'
        if len( class_name ) > 15:
            class_font_size = '8px'
            class_padding = '4px'
        race_dict = self.fields['Race']

        filename = ''
        ext = ''
        race_portrait_path = 'systems/OSRIC/portraits/Races/'
        valid_images = [ ".jpg", ".gif", ".png", ".jpeg" ]
        for f in os.listdir( race_portrait_path ):
            f_split = os.path.splitext( f )
            f_root = f_split[0]
            f_ext = f_split[1]
            if race_dict['unique_id'] == f_root and f_ext.lower() in valid_images:
                    filename = f
                    ext = f_ext
                    break
        if ext == 'jpeg':
            ext = 'jpg'
        with open( os.path.join( race_portrait_path, filename ) ) as portrait_file:
            portrait = base64.b64encode( portrait_file.read() )

        equipment_list = self.fields['EquipmentList']
        proficiency_list = self.fields['ProficiencyList']
        class_abilities = {}
        if 'classes' in class_dict:
            for cl in class_dict['classes']:
                class_abilities[ cl['Name'] ] = SystemSettings.get_class_abilities( level, self.attr_cache, cl )
        else:
            class_abilities[ class_dict['Name'] ] = SystemSettings.get_class_abilities( level, self.attr_cache, class_dict )
        race_abilities = SystemSettings.get_race_abilities( race_dict )
        spellbook = self.fields['SpellbookList']
        spells_daily = self.fields['SpellsList']
        spells2_daily = self.fields['Spells2List']
        #print equipment_list
        saves_dict = SystemSettings.get_saves( 1, self.attr_cache, class_dict, race_dict )
        money_dict = SystemSettings.get_coinage_from_float( self.pages['EquipmentPage'].slots_remaining )
        #print money_dict
        movement_tuple = SystemSettings.calculate_movement( race_dict, class_dict, self.attr_cache, equipment_list )
        markup_template_dict = {
            'class_font_size': class_font_size,
            'class_padding': class_padding,
            'name': self.fields['Name'],
            'gender': self.fields['Gender'],
            'class': class_dict['Name'],
            'alignment': self.fields['Alignment'],
            'race': race_dict['Name'],
            'xp': 0,
            'hp': self.hp_cache,
            'ac': SystemSettings.calculate_ac( self.attr_cache, class_dict, race_dict, equipment_list ),
            'level': 1,
            'age': self.age_cache,
            'height': str( self.height_weight_cache[0][0] ) + 'ft ' + str( self.height_weight_cache[0][1] ) + 'in',
            'weight': str( self.height_weight_cache[1] ) + ' lbs',
            'portrait': portrait,
            'image_type': ext,
            'tohit_row': '<td align=center>' + '</td><td align=center>'.join( SystemSettings.get_tohit_row( 1, class_dict, race_dict ) ) + '</td>',
            'movement_rate': movement_tuple[0],
            'movement_desc': movement_tuple[1],
            }
        for attr_name in list( self.attr_cache.keys() ):
            markup_template_dict[attr_name] = self.attr_cache[attr_name]
            markup_template_dict[ attr_name + '_bonus' ] = self.get_attr_bonuses_string( attr_name, self.attr_cache[attr_name] )
        for save in list( saves_dict.keys() ):
            markup_template_dict[save] = saves_dict[save]
        for denomination in list( money_dict.keys() ):
            markup_template_dict[denomination] = money_dict[denomination]

        markup = '''\
<style type=text/css>
.border {
    color: red;
    border-style: solid;
    border-color: purple;
    margin-right: 5px;
}

.bigger-font {
    font-size: 15px;
}

.smaller-font {
    font-size: 10px;
}

.pad-cell {
    padding-left: 5px;
    padding-right: 5px;
}

.pad-bottom {
    padding-bottom: 5px;
}

.pad-all {
    padding: 5px;
}

.no-pad {
    padding: 0px;
}

.lpad {
    padding-left: 15px;
}

.float-right {
    float: right;
}

.class-font {
    font-size: $class_font_size;
    padding-top: $class_padding;
}

.alignment-font {
    font-size: 10px;
    padding-top: 3px;
}

.attr-bonuses {
    font-size: 8px;
    vertical-align: middle;
    white-space: pre;
}

.tohit-table {
    border-style: solid;
}

.tohit-table > tr > td {
    padding: 2px;
    vertical-align: middle;
}

.equipment-table > tr > th {
    padding: 4px;
    align: center;
    font-size: 10px;
}

.equipment-table > tr > td {
    padding: 4px;
    align: center;
    font-size: 10px;
}

.equip-legend {
    font-size: 8px;
}

table.ability {
    font-size: 12px;
}

table.ability > tr > th {
    padding: 2px;
}

.pre {
    white-space: pre;
}

p.page-break {
    page-break-after:always;
}
</style>
<h1 align=center>$name</h1>
<img class=float-right align=right height=140 src=data:image/$image_type;base64,$portrait</img>

<table>
<tr><td class=pad-bottom><b>Name: </b></td><td align=right>$name</td><td class=lpad><b>XP: </b></td><td align=right>$xp</td><td class=lpad><b>Age: </b></td align=right><td align=right>$age</td></tr>
<tr><td class=pad-bottom><b>Class: </b></td><td align=right class=class-font>$class</td><td class=lpad><b>HP: </b></td><td align=right>$hp</td><td class=lpad><b>Height: </b></td><td align=right>$height</td></tr>
<tr><td class=pad-bottom><b>Alignment: </b></td><td align=right class=alignment-font>$alignment</td><td class=lpad><b>AC: </b></td><td align=right>$ac</td><td class=lpad><b>Weight: </b></td><td align=right>$weight</td></tr>
<tr><td class=pad-bottom><b>Race: </b></td><td align=right>$race</td><td class=lpad><b>Level: </b></td><td align=right>$level</td><td class=lpad><b>Gender: </b></td><td align=right>$gender</td></tr>
</table>

<hr />

<table align=center><tr>

<td>
<table class='border bigger-font' border=2 ><tr><td>
<table class=pad-cell>
<tr><td align=right class=pad-cell>Str:</td><td align=right class=pad-cell>$STR</td><td class=attr-bonuses> $STR_bonus </td></tr>
<tr><td align=right class=pad-cell>Int:</td><td align=right class=pad-cell>$INT</td><td class=attr-bonuses> $INT_bonus </td></tr>
<tr><td align=right class=pad-cell>Wis:</td><td align=right class=pad-cell>$WIS</td><td class=attr-bonuses> $WIS_bonus </td></tr>
<tr><td align=right class=pad-cell>Dex:</td><td align=right class=pad-cell>$DEX</td><td class=attr-bonuses> $DEX_bonus </td></tr>
<tr><td align=right class=pad-cell>Con:</td><td align=right class=pad-cell>$CON</td><td class=attr-bonuses> $CON_bonus  </td></tr>
<tr><td align=right class=pad-cell>Cha:</td><td align=right class=pad-cell>$CHA</td><td class=attr-bonuses> $CHA_bonus </td></tr>
</table>
</td></tr></table>
</td>

<td>
<table class=smaller-font align=center border=1>
<tr><td colspan=2><h3 align=center>Saving Throws</h3></td></tr>
<tr><td class=pad-cell>Aimed Magic Items</td><td class=pad-cell align=right>$Aimed_Magic_Items </td></tr>
<tr><td class=pad-cell>Breath Weapon</td><td class=pad-cell align=right>$Breath_Weapons </td></tr>
<tr><td class=pad-cell>Death, Paralysis, Poison</td><td class=pad-cell align=right>$Death_Paralysis_Poison </td></tr>
<tr><td class=pad-cell>Petrifaction, Polymorph</td><td class=pad-cell align=right>$Petrifaction_Polymorph </td></tr>
<tr><td class=pad-cell>Spells</td><td class=pad-cell align=right>$Spells </td></tr>
</tr></table>
</td>

</tr></table>

<hr />

<table class=tohit-table border=1 align=center>
<tr><td><b>Enemy AC</b></td><td align=center> -10 </td><td align=center> -9 </td><td align=center> -8 </td><td align=center> -7 </td><td align=center> -6 </td><td align=center> -5 </td>
    <td align=center> -4 </td><td align=center> -3 </td><td align=center> -2 </td><td align=center> -1 </td><td align=center> 0 </td><td align=center> 1 </td>
    <td align=center> 2 </td><td align=center> 3 </td><td align=center> 4 </td><td align=center> 5 </td><td align=center> 6 </td><td align=center> 7 </td>
    <td align=center> 8 </td><td align=center> 9 </td><td align=center> 10 </td></tr>
<tr><td><b>To Hit</b></td>$tohit_row</tr>
</table>

<hr />

<div class=pre align=center>GP: $gp     PP: $pp     EP: $ep     SP: $sp     CP: $cp</div>

<hr />

<table align=center>
<tr><th><h4>Equipment</h4></th></tr>
<tr><td><table border=1 class=equipment-table>
<tr><th>Name</th><th>Damage Vs S or M</th><th>Damage Vs L</th><th>Damage Type</th><th>RoF</th><th>Range</th><th>Max Move</th><th>AC Effect</th><th>Notes</th></tr>
'''

        for equip in equipment_list:
            equip_name = equip['Name']
            if equip in proficiency_list:
                equip_name = equip_name + '*'
            equip_list = [equip_name, equip['Damage_Vs_S_or_M'], equip['Damage_Vs_L'], equip['Damage_Type'],
                           equip['Rate_of_Fire'], equip['Range'], equip['Max_Move_Rate'], str(equip['AC_Effect']), equip['Notes']]
            markup += '<tr><td align=center>' + '</td><td align=center>'.join(equip_list) + '</td></tr>'

        markup += '''
</table></td></tr></table>
<div align=center class=equip-legend>*=Proficient</div>
<div><b>Movement Rate: </b>$movement_rate ft/round<br />
<b>Surprise: </b>$movement_desc</div>

<p class=page-break></p>
<h2>Ablities</h2>
'''

        if class_abilities:
            for cl in list( class_abilities.keys() ):
                markup += '\n<h5>{} Abilities</h5>\n'.format( cl )
                for i, a in enumerate( class_abilities[cl] ):
                    if a[0]:
                        if i > 0:
                            markup += '<br />'
                        markup += '<b>{}: </b>{}\n'.format( *a )
                    else:
                        markup += '<table class=ability align=center border=1>\n<tr>'
                        for h in a[1][0]:
                            markup += '<th align=center>{}</th>'.format(h)
                        markup += '</tr>\n<tr>'
                        for d in a[1][1]:
                            markup += '<td align=center>{}</td>'.format(d)
                        markup += '</tr>\n</table>\n'

        if race_abilities:
            markup += '\n<h5>{} Abilites</h5>\n'.format( race_dict['Name'] )
            markup += '<ul>\n'
            for a in race_abilities:
                markup += '<li>'
                markup += a[0]
                if a[1]:
                    markup += ' {}'.format( a[1] )
                if a[2]:
                    markup += ' {}'.format( a[2] )
                markup += '</li>\n'
            markup += '</ul>\n'

        spellcaster = False
        if 'classes' in class_dict:
            for cl in class_dict['classes']:
                if SystemSettings.has_spells_at_level( level, cl ):
                    spellcaster = True
        else:
            if SystemSettings.has_spells_at_level( level, class_dict ):
                spellcaster = True

        if spellcaster:
            spell_item_string = '''
<h3>{Name}</h3>
<b>Reversible: </b>{Reversible}<br/>
<b>Level: </b>{Level}<br />
<b>Damage: </b>{Damage}<br />
<b>Range: </b>{Range}<br />
<b>Duration: </b>{Duration}<br />
<b>Area of Effect: </b>{Area_of_Effect}<br />
<b>Components: </b>{Components}<br />
<b>Casting Time: </b>{Casting_Time}<br />
<b>Saving Throw: </b>{Saving_Throw}<br />
<b>Description: </b><span class=pre>{Description}</span><br /><br />
'''
            markup += '<p class=page-break></p>\n<h2>Spells</h2>\n'
            if spellbook:
                markup += '<h5>Spellbook</h5>\n<hr />'
                for spell in spellbook:
                    markup += spell_item_string.format( **spell )
                markup += '<hr />\n'
            if spells_daily:
                markup += '<h5>{} Daily Spells</h5>\n<hr />'.format( spells_daily[0]['Type'].title().replace( '_', ' ' ) )
                for spell in spells_daily:
                    markup += spell_item_string.format( **spell )
                markup += '<hr />\n'
            if spells2_daily:
                markup += '<h5>{} Daily Spells</h5>\n<hr />'.format( spells2_daily[0]['Type'].title().replace( '_', ' ' ) )
                for spell in spells2_daily:
                    markup += spell_item_string.format( **spell )
                markup += '<hr />\n'

        t = Template(markup)
        return t.safe_substitute( markup_template_dict )
