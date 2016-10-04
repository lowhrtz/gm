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
    content = [
        ('text', 'How do you want to create your character?'),
        ('fillHook', 'get_choices', None, 'radiobutton', 'index'),
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
    template = "InfoPage"
    content = [
        ('listbox', 'Race', 'method', 'get_available_races', '^$ WP{attributes} DB{races}'),
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

class ChooseClassPage(WizardPage):
    page_title = "Choose Class"
    page_subtitle = "Choose from the available classes"
    page_id = 50
    template = "infopage"
    layout = "Horizontal"
    content = [
        ('text', 'Here are the available classes.'),
        ('listbox', 'Class', 'method', 'get_available_classes', '^$ WP{attributes} DB{classes} F{Race} DB{races_meta}'),
#        ('choose', 'Class', 'method', 'get_available_classes', '^$ WP{attributes} DB{classes} F{Race} DB{races_meta}'),
    ]

    def get_available_classes(self, attribute_dict, class_dict_list, race, race_meta_dict_list):
        all_normal_classes = [cl['Name'] for cl in class_dict_list]
        if attribute_dict is None and race == '':
            return all_normal_classes

        class_option_list = None
        for race_meta_dict in race_meta_dict_list:
            if race_meta_dict['race_id'] == race['unique_id'] and race_meta_dict['Type'] == 'class' and race_meta_dict['Subtype'] == 'permitted class options':
                class_options = race_meta_dict['Modified']
                class_option_list = [class_option.strip() for class_option in class_options.split(',')]

        if not class_option_list:
            class_option_list = all_normal_classes

        allowed_list = []
        if attribute_dict is not None:
            attribute_dict = {k.lower():int(v) for k, v in attribute_dict.items()}

        allowed_normal_classes = [normal_class['Name'] for normal_class in class_dict_list if self.class_allowed(normal_class, attribute_dict)]
        for class_option in class_option_list:
            class_option_allowed = True
            for class_option_item in class_option.split('/'):
                class_option_item = class_option_item.strip()
                if class_option_item not in allowed_normal_classes:
                    class_option_allowed = False
            if class_option_allowed:
                allowed_list.append(class_option)

        return allowed_list

    def class_allowed(self, cl, attribute_dict):
        allowed = True
        minimum_scores = [i.strip() for i in cl['Minimum_Scores'].split(',')]
        for score in minimum_scores:
            score_key = score[:3].lower()
            score_value = int(score[3:].strip())
            if attribute_dict is not None and attribute_dict[score_key] < score_value:
                allowed = False
        return allowed

class ReviewPage(WizardPage):
    page_title = "Review"
    page_subtitle = "Review Subtitle"
    page_id = 60
    content = "This is the review content."
