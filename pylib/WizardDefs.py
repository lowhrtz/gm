# -*- coding: utf-8 -*-

class WizardPage(object):
    page_title = None
    page_subtitle = None
    page_id = None
    layout = "vertical"
    template = "default"
    content = None
    
    def __str__(self):
        return self.page_title

    def get_page_title(self):
        return self.__str__()

    def get_page_subtitle(self):
        return self.page_subtitle


    def get_page_id(self):
        return self.page_id

    def get_next_page_id(self):
#        return self.page_id +1
        return -2


    def get_layout(self):
        return self.layout

    def get_template(self):
        return self.template
        
    def get_content(self):
        return self.content
