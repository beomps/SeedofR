from django import forms

from .models import Data

#이곳에 프론트에서 사용되는 폼을 정의합니다.
class PostForm(forms.ModelForm):

    #실제로는 아래보다 모델에 있는 파라미터를 더 입력 받아햐 합니다.
    class Meta:
        model = Data
        fields = ('plant_name', 'plant_picture','temp', 'humid', 'light','flow_water','ph_water','tds_water','started_date','published_date','etc')
