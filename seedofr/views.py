from django.shortcuts import get_object_or_404, render, redirect
from .models import Data
from django.http import HttpResponse
from .forms import PostForm
from django.utils import timezone
from watson_developer_cloud import VisualRecognitionV3
import json

def index(request):
    return render(request, 'seedofr/index.html')

# 아래에 api 입력 및 로직 구현합니다.
def post_new(request):

    visual_recognition = VisualRecognitionV3(
        '2018-03-19',
        iam_apikey='XPidzig_QHPkej4yOM1_558gdA-uU6J-C0P8mPdKQB9c'
    )
    """
    with open('./fruitbowl.jpg', 'rb') as images_file:
        classes = visual_recognition.classify(
            images_file,
            threshold='0.6',
        classifier_ids='default').get_result()
    
    print(json.dumps(classes, indent=2))        
    """    
    if request.method == "POST":
        form = PostForm(request.POST, request.FILES)
        if form.is_valid():
            post = form.save(commit=False)
            post.author = request.user
            post.published_date = timezone.now()
            uploadFile = request.FILES['plant_picture'];

            
            classes = visual_recognition.classify(
                uploadFile,
                threshold='0.6',
            classifier_ids='default').get_result()
            
            print(json.dumps(classes, indent=2))  
            

            post.save()
            return redirect('post_detail', seq=post.seq)
    else:
        form = PostForm()
    return render(request, 'seedofr/post_edit.html', {'form': form})

def post_detail(request, seq):
    data = get_object_or_404(Data, seq=seq)
    return render(request, 'seedofr/post_detail.html', {'data': data})

def post_compare(request):
    form = PostForm()
    return render(request, 'seedofr/post_compare.html', {'form': form})

def post_list(request):
    datas = Data.objects.filter(published_date__lte=timezone.now()).order_by('published_date')
    return render(request, 'seedofr/post_list.html', {'datas': datas})