<?php

use Nette\Application\UI\Form;
/*
 * clanky presenter
 */

class ClankyPresenter extends BasePresenter
{
	
    private $clankyRepository;
    private $record;
	
    protected function startup()
    {
		parent::startup();
		$this->clankyRepository = $this->context->clankyRepository;
    }

    public function renderDefault()
    {
		$this->template->clanky = $this->clankyRepository->findAll()->order('ID_clanku DESC');
    }
	
    var $detailClanku;
    public function actionDetail($id) {
	$this->detailClanku = $this->clankyRepository->findBy(array('ID_clanku' => $id))->fetch();
	if ($this->detailClanku === FALSE) {
	    $this->setView('notFound');
	}
    }

    public function actionEditclanek($id_clanku) {
	if (!$this->getUser()->isInRole('admin')) {
	    $this->flashMessage('Pro vykonání této akce nemáte dostatečná práva', 'error');
	    $this->redirect('Clanky:');
	}

	$this->record = $this->clankyRepository->findBy(array('ID_clanku' => $id_clanku))->fetch();
	if (!$this->record) {
	    $this->setView('notFound');
	}
	$this['editClanekForm']->setDefaults(array(
	    'nazev' => $this->record['Nazev'],
	    'uvod' => $this->record['Uvod'],
	    'text' => $this->record['Text'],
	    'autor' => $this->record['Autor'],
	    'datum' => $this->record['Datum']
	));
    }

    public function actionAddclanek($id_clanku) {
	if (!$this->getUser()->isInRole('admin')) {
	    $this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
	    $this->redirect('Clanky:');
	}
	$this['addClanekForm']->setDefaults(array(
	    'autor' => $this->getUser()->getIdentity()->fullname
	));
    }

    public function renderDetail() {
		$this->template->clanek = $this->detailClanku;
    }

    /*
     * pridani clanku
     */
    protected function createComponentAddClanekForm()
    {
	$form = new Form();
	$form->addTextArea('nazev', 'Nazev:', 50, 2)
		->setRequired('Zadejte prosím název článku');
	$form->addTextArea('uvod', 'Úvod:', 50, 6)
	    ->addRule(Form::MAX_LENGTH, 'Maximální délka popisu je 400 znaku', 400);
	$form->addTextArea('text', 'Text:', 70, 30)
	    ->addRule(Form::MAX_LENGTH, 'Maximální délka popisu je 5000 znaku', 5000)
		->setRequired('Zadejte prosím text článku');;
	$form->addText('autor', 'Autor:')
		->setRequired('Zadejte prosím autora článku');;
	$form->addSubmit('addAkce', 'Přidat')
	    ->onClick[] = $this->addClanekFormSubmitted;
	$form->addSubmit('cancel', 'Storno')
	    ->setValidationScope(FALSE)
	    ->onClick[] = callback($this, 'clanekFormCancel');
	$form->setMethod('POST');
	return $form;
    }

    public function addClanekFormSubmitted(Nette\Forms\Controls\SubmitButton $btn) {
	$values=$btn->form->getValues();
	try {
	    $this->clankyRepository->addClanek($values);
	    $this->flashMessage('Operace byla úspěšně provedena', 'success');
	    $this->redirect('Clanky:default');
	} catch (Exception $e) {
	    if (!($e instanceof Nette\Application\AbortException)) {
		$this->flashMessage('Operaci nebylo možné provést', 'error');
	    }
	    $this->redirect('Clanky:default');
	}
    }

    public function clanekFormCancel(Nette\Forms\Controls\SubmitButton $btn) {
	$this->redirect('Clanky:default');
    }

    /*
     * editace clanku
     */
    protected function createComponentEditClanekForm()
    {
	$form = new Form();
	$form->addTextArea('nazev', 'Nazev:', 50, 2)
		->setRequired('Zadejte prosím název článku');
	$form->addTextArea('uvod', 'Úvod:', 50, 6)
	    ->addRule(Form::MAX_LENGTH, 'Maximální délka popisu je 400 znaku', 400);
	$form->addTextArea('text', 'Text:', 70, 30)
	    ->addRule(Form::MAX_LENGTH, 'Maximální délka popisu je 5000 znaku', 5000)
		->setRequired('Zadejte prosím text článku');
	$form->addText('autor', 'Autor:')
		->setRequired('Zadejte prosím autora článku');
	$form->addText('datum', 'Datum:');
	$form->addSubmit('addAkce', 'Potvrdit změny')
	    ->onClick[] = $this->editClanekFormSubmitted;
	$form->addSubmit('cancel', 'Storno')
	    ->setValidationScope(FALSE)
	    ->onClick[] = callback($this, 'editClanekFormCancel');
	$form->setMethod('POST');
	return $form;
    }

    public function editClanekFormSubmitted(Nette\Forms\Controls\SubmitButton $btn) {
	$values=$btn->form->getValues();
	try {
	    $this->clankyRepository->getConnection()->exec(
		'UPDATE clanky SET Nazev=?, Uvod=?, Text=?, Autor=?, WHERE ID_clanku=?',
		$values->nazev, $values->uvod, $values->text, $values->autor, $this->record['ID_clanku']);
	    $this->flashMessage('Operace byla úspěšně provedena', 'success');
	    $this->redirect('Clanky:detail', $this->record['ID_clanku']);
	} catch (Exception $e) {
	    if (!($e instanceof Nette\Application\AbortException)) {
		$this->flashMessage('Operaci nebylo možné provést', 'error');
	    }
	    $this->redirect('Clanky:detail', $this->record['ID_clanku']);
	}
    }

    public function editClanekFormCancel(Nette\Forms\Controls\SubmitButton $btn) {
	$this->redirect('Clanky:detail', $this->record['ID_clanku']);
    }

    /*
     * odstraneni článku
     */
    public function deleteClanek($id) {
	$this->clankyRepository->deleteClanek($id);
	$this->flashMessage('Článek byl úspěšně odstranen z databáze', 'success');
	$this->redirect('Clanky:default');
    }

}
