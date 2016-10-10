# -*- coding: utf-8 -*-
from WizardDefs import WizardPage
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
    #    The _ tells GM to hide the field name from view; it still acessable by that name minus the _
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
        ('listbox-determinesNext', 'Class_', 'method', 'get_available_classes', '^$ WP{attributes} DB{classes} F{Race} DB{races_meta}', '^$ F{Class} DB{classes_meta.cols(class_id, Type, Level, Casting_Level).where(class_id=Class:unique_id)}'),
#        ('choose', 'Class', 'method', 'get_available_classes', '^$ WP{attributes} DB{classes} F{Race} DB{races_meta}'),
    ]

    def get_available_classes(self, attribute_dict, class_dict_list, race, race_meta_dict_list):
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
                            'classes':[],
                        }
                        name_list = []
                        for cl in class_option.split('/'):
                            class_record = self.find_class_record(cl, class_dict_list)
                            name_list.append(class_record['Name'])
                            multiclass_dict['classes'].append(class_record)
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

    def get_next_page_id(self, class_dict={}, class_meta_dict={}):
        print class_meta_dict
        spellcaster = False
        if 'classes' in class_dict:
            for cl in class_dict['classes']:
#                print len(class_dict['meta_table'])
                if cl['Primary_Spell_List'] != 'None':
                    spellcaster = True
        else:
#            print len(class_dict['meta_table'])
            if class_dict['Primary_Spell_List'] != 'None':
                spellcaster = True
        if spellcaster:
            return SpellsPage.page_id
        return ReviewPage.page_id

class SpellsPage(WizardPage):
#    enabled = False
    page_title = "Choose Spells"
    page_subtitle = "Choose the spells for your character."
    page_id = 55
    layout = "horizontal"
    template = "DualListPage"
    content = ""

class ReviewPage(WizardPage):
    page_title = "Review"
    page_subtitle = "Review Subtitle"
    page_id = 60
    content = "This is the review content."
