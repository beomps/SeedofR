from django.shortcuts import get_object_or_404, render, redirect
from .models import Data
from django.http import HttpResponse
from .forms import PostForm
from django.utils import timezone


def index(request):
    return render(request, 'seedofr/index.html')

# 아래에 api 입력 및 로직 구현합니다.
def post_new(request):
    if request.method == "POST":
        form = PostForm(request.POST)
        if form.is_valid():
            post = form.save(commit=False)
            post.author = request.user
            post.published_date = timezone.now()
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